#include "selfma_api.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include "container.h"
#include "error_handler.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "selfma_file.h"
#include "task.h"

typedef struct selfma_opq {
    std::unique_ptr<Container> container;
    std::fstream fd;
    char uuid[MAX_NAME_LENGTH];
    char user_data[MAX_DESCRIPTION_LENGTH];
} selfma_ctx_t;

typedef struct {
    char project[sizeof(Project)];
    Task* arr;
} selfma_fformat_t;

typedef struct {
    selfma_fformat_t data;
    size_t num_bytes;
} chunk_info_t;

typedef struct {
    uint32_t version;                          // File format version
    char magic[4];                             // Magic number for file identification
    char container_id[MAX_NAME_LENGTH];        // User identification (SHA256)
    char user_buffer[MAX_DESCRIPTION_LENGTH];  // user buffer
    size_t user_data_length;                   // Length of user data (max 1024 bytes)
    uint8_t chunk_num;                         // Number of main tree nodes
    chunk_info_t* chunk_info;
} header_t;

static VoidResult serialize(selfma_ctx_t* ctx) {
    if (!is_storage()) {
        return Err(ErrorCode::NO_STORAGE, "No storage available");
    }

    header_t header = {0};
    header.version = HEADER_VERSION;

    std::vector<chunk_info_t> chunks;
    uint32_t needed_size = 0;
    auto projects = ctx->container->to_vector();
    needed_size += (projects.size() * sizeof(Project));  // add amount of projects objects

    for (auto project : projects) {
        auto tree = project->to_vector();
        chunk_info_t chunk;
        chunk.num_bytes = sizeof(Task) * tree.size();
        chunk.data.arr = (Task*) malloc(sizeof(Task) * tree.size());

        memcpy(&chunk.data.project, project, sizeof(Project));

        for (size_t i = 0; i < tree.size(); i++) {
            memcpy(&chunk.data.arr[i], tree[i], sizeof(Task));
        }
        chunks.push_back(chunk);
        needed_size += chunk.num_bytes;
    }

    // set header
    header.chunk_info = (chunk_info_t*) malloc(needed_size);
    header.chunk_num = chunks.size();
    strncpy(header.container_id, ctx->uuid, MAX_NAME_LENGTH);
    strncpy(header.user_buffer, ctx->user_data, MAX_DESCRIPTION_LENGTH);
    header.user_data_length = MAX_DESCRIPTION_LENGTH;
    memcpy(header.magic, "FACA", 4);
    memcpy(header.chunk_info, chunks.data(), chunks.size() * sizeof(chunk_info_t));

    // write to file
    auto endpoint = hash_to_file(ctx->uuid);
    if (!is_exist(endpoint)) {
        QWISTYS_TODO_MSG("Handle new sdb file");
        // return Err(ErrorCode::FILE_NOT_FOUND, "File does not exist");
    }

    std::ofstream file(endpoint, std::ios::binary | std::ios::trunc);
    if (!file) {
        return Err(ErrorCode::FILE_OPEN_ERROR, "Failed to open file for writing");
    }

    // Write header
    if (!file.write(reinterpret_cast<const char*>(&header), sizeof(header_t))) {
        return Err(ErrorCode::WRITE_ERROR, "Failed to write header");
    }

    // Write chunk information
    if (!file.write(reinterpret_cast<const char*>(header.chunk_info), header.chunk_num * sizeof(chunk_info_t))) {
        return Err(ErrorCode::WRITE_ERROR, "Failed to write chunk info");
    }

    // Write chunks data
    for (const auto& chunk : chunks) {
        if (!file.write(reinterpret_cast<const char*>(chunk.data.arr), chunk.num_bytes)) {
            return Err(ErrorCode::WRITE_ERROR, "Failed to write chunk data");
        }
    }

    // release
    for (auto& chunk : chunks) {
        free(chunk.data.arr);
    }
    free(header.chunk_info);

    return Ok();
}

API_SELFMA VoidResult selfma_serialize(selfma_ctx_t* ctx) {
        return serialize(ctx);
}

static VoidResult deserialize(selfma_ctx_t* ctx) {
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
#if 0    // Read and process each chunk
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
        return deserialize(ctx);
}

API_SELFMA selfma_ctx_t* selfma_create(uint32_t id, const char* container_id, const char* user_buffer) {
    selfma_ctx_t* ctx = (selfma_ctx_t*) qwistys_malloc(sizeof(selfma_ctx_t), nullptr);
    if (ctx) {
        ctx->container = std::make_unique<Container>();
        memcpy(ctx->uuid, container_id, MAX_NAME_LENGTH);
        memcpy(ctx->user_data, user_buffer, MAX_NAME_LENGTH);
    }
    return ctx;
}

API_SELFMA void selfma_destroy(selfma_ctx_t* ctx) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        ctx->container.reset();
        qwistys_free(ctx);
    }
}

API_SELFMA VoidResult selfma_add_project(selfma_ctx_t* ctx, const char* name, const char* description) {
    if (ctx) {
        ProjConf config(0, name, description);
        return ctx->container->add_project(config);
    }
    return Err(ErrorCode::ADD_PROJECT_FAIL, "Fail to add to the project", Severity::LOW);
}

API_SELFMA VoidResult selfma_remove_project(selfma_ctx_t* ctx, uint32_t id) {
    if (ctx) {
        return ctx->container->remove_project(id);;
    }
    return Ok();
}

API_SELFMA VoidResult selfma_add_task(selfma_ctx_t* ctx, uint32_t project_id, const char* name, const char* description) {
    if (ctx) {
        QWISTYS_TODO_MSG("Need to make a decision about duration values. >;-()");
        TaskConf_t conf = {0};
        conf.description = (char*) malloc(MAX_DESCRIPTION_LENGTH);
        if (!conf.description) {
            return Err(ErrorCode::ADD_TASK_FAIL, "Fail to add a task", Severity::LOW);
        }

        strncpy(conf.description, description, MAX_DESCRIPTION_LENGTH - 1);
        conf.description[MAX_DESCRIPTION_LENGTH - 1] = '\0';  // Ensure null-termination
        QWISTYS_TODO_MSG("What taks structs hold in? need to add more info ?");
        // // Similarly for the name field if it exists
        // if (conf.name) {
        //     strncpy(conf.name, name, MAX_NAME_LENGTH - 1);
        //     conf.name[MAX_NAME_LENGTH - 1] = '\0';
        // }

        Task t(&conf);
        ctx->container->add_task(project_id, &t);

        free(conf.description);
        return Ok();
    }
    return Err(ErrorCode::ADD_TASK_FAIL, "Fail to add a task", Severity::LOW);
}

API_SELFMA VoidResult selfma_remove_task(selfma_ctx_t* ctx, uint32_t project_id, uint32_t task_id) {
    if (ctx) {
        return ctx->container->remove_task(project_id, task_id);
    }
    return Err(ErrorCode::INPUT, "ctx is null", Severity::LOW);
}

API_SELFMA void selfma_print(selfma_ctx_t* ctx) {
    if (ctx) {
        ctx->container->print();
    };
}
