#include <string.h>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include "container.h"
#include "error_handler.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "selfma_api.h"
#include "selfma_file.h"
#include "task.h"

static inline void copy_strchar(const std::string& src, char* buffer, size_t MAX_DEFINES) {
    auto view = std::string_view(src);
    auto length = QWISTYS_MIN(view.length(), MAX_DEFINES - 1);
    std::copy_n(view.begin(), length, buffer);
    buffer[length] = '\0';
}

typedef struct selfma_opq {
    // std::unique_ptr<Container> container;
    Container* container;
    std::fstream fd;
    char uuid[MAX_NAME_LENGTH];
    char* user_data;
} selfma_ctx_t;

typedef struct {
    char project[sizeof(Project)];
    Task arr[];
} selfma_fformat_t;

selfma_fformat_t* get_format_buffer(size_t num_of_items) {
    return (selfma_fformat_t*) qwistys_malloc(sizeof(selfma_fformat_t) + (sizeof(Task) * num_of_items), nullptr);
}

typedef struct {
    uint32_t crc;                              // Do i need to check the integrety of data from storaje ?
    uint32_t version;                          // File format version
    char magic[4];                             // Magic number for file identification
    uint8_t num_of_chunks;                     // Number of main tree nodes
    size_t user_data_length;                   // Length of user data (max 1024 bytes)
    char user_buffer[MAX_DESCRIPTION_LENGTH];  // user buffer
    char file_name[MAX_NAME_LENGTH];           // user buffer
    uint32_t each_chunk_size[];                // User identification (SHA256)
} header_t;

size_t header_size(size_t num_of_chunks) {
    return sizeof(header_t) + sizeof(uint32_t) * num_of_chunks;
}

header_t* get_header_buffer(size_t num_of_chunks) {
    return (header_t*) qwistys_malloc(header_size(num_of_chunks), nullptr);
}

static VoidResult serialize(selfma_ctx_t* ctx) {
    auto ret = Ok();
    if (!is_storage()) {
        return Err(ErrorCode::NO_STORAGE, "No storage available");
    }

    header_t* header = nullptr;

    auto projects = ctx->container->to_vector();

    header = get_header_buffer(projects.size());
    QWISTYS_ASSERT(header);

    // Get the size of amount of chuns. in different How many "Project" tree you have/ and how long each "Task" tree.
    header->num_of_chunks = projects.size();
    for (auto i = 0; i < header->num_of_chunks; i++) {
        auto tree = projects[i]->to_vector();
        header->each_chunk_size[i] = tree.size();
    }

    // set header
    strncpy(header->file_name, hash_to_file(ctx->uuid).c_str(),
            QWISTYS_MIN(strlen(hash_to_file(ctx->uuid).c_str()), MAX_NAME_LENGTH - 1));
    strncpy(header->user_buffer, ctx->user_data, QWISTYS_MIN(strlen(ctx->user_data), MAX_DESCRIPTION_LENGTH));
    memcpy(header->magic, "FACA", 4);

    // Open/Create file
    std::ofstream file(header->file_name, std::ios::binary | std::ios::trunc);
    if (!file) {
        qwistys_free(header);
        return Err(ErrorCode::FILE_OPEN_ERROR, "Failed to open file for writing");
    }

    // Write header
    if (!file.write(reinterpret_cast<const char*>(header), header_size(header->num_of_chunks))) {
        return Err(ErrorCode::WRITE_ERROR, "Failed to write header");
    }

    // write to data
    for (int i = 0; i < projects.size(); i++) {
        auto task_data = projects[i]->to_vector();

        if (!file.write(reinterpret_cast<const char*>(projects[i]), sizeof(Project))) {
            ret = Err(ErrorCode::WRITE_ERROR, "Failed to write header");
            break;
        }
        for (auto task : task_data) {
            if (!file.write(reinterpret_cast<const char*>(task), sizeof(Task))) {
                ret = Err(ErrorCode::WRITE_ERROR, "Failed to write header");
                break;
            }
        }
    }
    qwistys_free(header);
    header = nullptr;
    file.close();
    return ret;
}

API_SELFMA VoidResult selfma_serialize(selfma_ctx_t* ctx) {
    if (ctx) {
        return serialize(ctx);
    }
    return Err(ErrorCode::INPUT, "Null ctx passed to serialization");
}

static VoidResult deserialize(selfma_ctx_t* ctx) {
#if 0  // Read and process each chunk
    if (!is_storage()) {
        return Err(ErrorCode::NO_STORAGE, "No storage available");
    }

    QWISTYS_DEBUG_MSG("storage");
    auto endpoint = hash_to_file(ctx->uuid);
    if (!is_exist(endpoint)) {
        return Err(ErrorCode::FILE_NOT_FOUND, "File does not exist");
    }

    std::ifstream file(endpoint, std::ios::binary);
    if (!file) {
        return Err(ErrorCode::FILE_OPEN_ERROR, "Failed to open file for reading");
    }

    // Read header
    header_t header;
    if (!file.read(reinterpret_cast<char*>(&header), sizeof(header_t))) {
        return Err(ErrorCode::READ_ERROR, "Failed to read header");
    }

    QWISTYS_DEBUG_MSG("read to header");
    // Verify magic number
    if (memcmp(header.magic, "FACA", 4) != 0) {
        return Err(ErrorCode::INVALID_FORMAT, "Invalid file format");
    }

    // Verify version
    if (header.version != HEADER_VERSION) {
        return Err(ErrorCode::VERSION_MISMATCH, "File version mismatch");
    }

    // Read chunk information
    std::vector<chunk_info_t> chunks(header.chunk_num);
    if (!file.read(reinterpret_cast<char*>(chunks.data()), header.chunk_num * sizeof(chunk_info_t))) {
        return Err(ErrorCode::READ_ERROR, "Failed to read chunk info");
    }

    // Clear existing data in ctx->container
    // ctx->container.reset(); // Not sure we need.
    QWISTYS_DEBUG_MSG("After check");
    for (const auto& chunk_info : chunks) {
        // Read project data
        // Create a new project in the container
        Project* p = (Project*) &chunk_info.data.project;
        ctx->container->add_project(p->config);
        auto proj = ctx->container->get_project(p->config._id);

        if (!proj) {
            return Err(ErrorCode::MEMORY_ERROR, "Failed to create new project");
        }

        // Read tasks
        std::vector<Task> tasks(chunk_info.num_bytes / sizeof(Task));
        if (!file.read(reinterpret_cast<char*>(tasks.data()), chunk_info.num_bytes)) {
            return Err(ErrorCode::READ_ERROR, "Failed to read task data");
        }

        // Add tasks to the project
        for (const auto& task : tasks) {
            QWISTYS_TODO_MSG("Created at value will be re-init, need to change this behavior also id will change");
            TaskConf_t conf = {0};
            memcpy(&conf.duration, &task.duration, sizeof(Duration));
            
            conf.description = (char*) malloc(MAX_DESCRIPTION_LENGTH);
            if (!conf.description) {
                return Err(ErrorCode::MEMORY_ERROR, "malloc fail", Severity::CRITICAL);
            }

            strncpy(conf.description, task.description, MAX_DESCRIPTION_LENGTH - 1);
            conf.description[MAX_DESCRIPTION_LENGTH - 1] = '\0';  // Ensure null-termination
            Task t(&conf);
            p->add(&t);
        }
    }

    // Update ctx with deserialized data
    strncpy(ctx->uuid, header.container_id, MAX_NAME_LENGTH);
    strncpy(ctx->user_data, header.user_buffer, MAX_DESCRIPTION_LENGTH);
#endif
    return Ok();
}

API_SELFMA VoidResult selfma_deserialize(selfma_ctx_t* ctx) {
    if (ctx) {
        return deserialize(ctx);
    }
    return Err(ErrorCode::INPUT, "Null ctx passed to deserialization");
}

API_SELFMA selfma_ctx_t* selfma_create(uint32_t id, const std::string& file_name, char* user_buffer) {
    selfma_ctx_t* ctx = (selfma_ctx_t*) qwistys_malloc(sizeof(selfma_ctx_t), nullptr);
    
    if (ctx) {
        QWISTYS_TODO_MSG("Handle windows case, for some reason smart pointers f@cked");
        // ctx->container = std::make_unique<Container>();
        ctx->container = new Container();
        copy_strchar(file_name, ctx->uuid, MAX_NAME_LENGTH);
        ctx->user_data = user_buffer;
    }
    return ctx;
}

API_SELFMA void selfma_destroy(selfma_ctx_t* ctx) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        // ctx->container.reset();
        delete ctx->container;
        qwistys_free(ctx);
    }
}

API_SELFMA VoidResult selfma_add_project(selfma_ctx_t* ctx, const std::string& name, const std::string& description) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        ProjConf config(0, name, description);
        return ctx->container->add_project(config);
    }
    return Err(ErrorCode::ADD_PROJECT_FAIL, "Fail to add to the project", Severity::LOW);
}

API_SELFMA VoidResult selfma_remove_project(selfma_ctx_t* ctx, uint32_t id) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        return ctx->container->remove_project(id);
    }
    return Ok();
}

API_SELFMA VoidResult selfma_add_task(selfma_ctx_t* ctx, uint32_t project_id, const std::string& name,
                                      const std::string& description, uint32_t duration) {
    auto ret = Ok();
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        TaskConf_t conf = {
            .description = description,
            .duration_in_sec = duration * 1.0f,
        };

        Task t(&conf);
        ret = ctx->container->add_task(project_id, &t);
    }
    return ret;
}

API_SELFMA VoidResult selfma_remove_task(selfma_ctx_t* ctx, uint32_t project_id, uint32_t task_id) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        return ctx->container->remove_task(project_id, task_id);
    }
    return Err(ErrorCode::INPUT, "ctx is null", Severity::LOW);
}

API_SELFMA void selfma_print(selfma_ctx_t* ctx) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        ctx->container->print();
    };
}

API_SELFMA void selfma_update(selfma_ctx_t* ctx) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        ctx->container->update();
    }
}
