#include <string.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "container.h"
#include "error_handler.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "selfma_api.h"
#include "selfma_file.h"
#include "task.h"

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

static void write_header(std::fstream& file, selfma_ctx_t* ctx, std::vector<Project*>& projects,
                         uint32_t uncompressed_data_size) {
    QWISTYS_TODO_MSG("Do i need to endian the data to file? to support stuff?");
    size_t hdr_size = sizeof(header_t) + projects.size() * sizeof(uint32_t);
    std::vector<char> header_buffer(hdr_size, 0);
    header_t* header = reinterpret_cast<header_t*>(header_buffer.data());

    QWISTYS_TODO_MSG("Calculate the CRC of the data");
    header->crc = 0xFFAAFFAA;  // Placeholder
    header->version = HEADER_VERSION;
    std::memcpy(header->magic, "SLFM", 4);
    header->num_of_chunks = static_cast<uint8_t>(projects.size());
    header->compression_type = static_cast<uint8_t>(CompressionType::NONE);
    header->uncompressed_data_size = uncompressed_data_size;

    // Handle user data
    if (ctx->user_data && strlen(ctx->user_data) > 0) {
        header->user_data_length = std::min(strlen(ctx->user_data), static_cast<size_t>(MAX_DESCRIPTION_LENGTH));
        std::strncpy(header->user_buffer, ctx->user_data, header->user_data_length);
    } else {
        header->user_data_length = 0;
    }

    // Handle file name
    std::string file_name = hash_to_file(ctx->uuid);
    std::strncpy(header->file_name, file_name.c_str(), MAX_NAME_LENGTH - 1);
    header->file_name[MAX_NAME_LENGTH - 1] = '\0';  // Ensure null-termination

    // Fill each_chunk_size array (number of tasks per project)
    for (size_t i = 0; i < projects.size(); ++i) {
        header->each_chunk_size[i] = projects[i]->to_vector().size();
    }

    // Write the entire header to file
    file.write(reinterpret_cast<const char*>(header), hdr_size);
}

static VoidResult serialize(selfma_ctx_t* ctx) {
    QWISTYS_TODO_MSG("TEST Serialization !!! not sure its working properly");
    QWISTYS_TELEMETRY_START();

    if (!is_storage()) {
        return Err(ErrorCode::NO_STORAGE, "No storage available");
    }

    auto projects = ctx->container->to_vector();

    // Collect all project/task data into a buffer
    std::vector<char> data_buffer;
    for (auto& project : projects) {
        const char* conf_ptr = reinterpret_cast<const char*>(&project->config);
        data_buffer.insert(data_buffer.end(), conf_ptr, conf_ptr + sizeof(ProjectConfigurations));
        auto tasks = project->to_vector();
        for (const auto& task : tasks) {
            const char* task_ptr = reinterpret_cast<const char*>(task);
            data_buffer.insert(data_buffer.end(), task_ptr, task_ptr + sizeof(Task));
        }
    }

    // Compress (stub: NONE — real algorithm can be swapped in later)
    std::vector<char> compressed;
    compress_data(CompressionType::NONE, data_buffer.data(), data_buffer.size(), compressed);

    // Open/Create file
    FileGuard endpoint(hash_to_file(ctx->uuid), std::ios::binary | std::ios::out | std::ios::trunc);
    if (!endpoint.is_open()) {
        QWISTYS_TELEMETRY_END();
        return Err(ErrorCode::FILE_OPEN_ERROR, "Failed to open file for writing");
    }

    write_header(endpoint.get(), ctx, projects, static_cast<uint32_t>(data_buffer.size()));
    endpoint.get().write(compressed.data(), compressed.size());

    QWISTYS_TELEMETRY_END();
    return Ok();
}

API_SELFMA VoidResult selfma_serialize(selfma_ctx_t* ctx) {
    if (ctx) {
        return serialize(ctx);
    }
    return Err(ErrorCode::INPUT, "Null ctx passed to serialization");
}

static VoidResult deserialize(const std::string& filename, selfma_ctx_t* ctx) {
    QWISTYS_TELEMETRY_START();
    if (!is_storage()) {
        return Err(ErrorCode::NO_STORAGE, "No storage available");
    }

    FileGuard endpoint(filename, std::ios::binary | std::ios::in);
    if (!endpoint.is_open()) {
        return Err(ErrorCode::FILE_OPEN_ERROR, "Failed to open file for reading");
    }

    header_t header;
    endpoint.get().read(reinterpret_cast<char*>(&header), sizeof(header_t));

    if (std::memcmp(header.magic, "SLFM", 4) != 0) {
        return Err(ErrorCode::INVALID_FORMAT, "Invalid file format");
    }

    QWISTYS_TODO_MSG("Implement crc check");
    std::vector<uint32_t> chunk_sizes(header.num_of_chunks);
    endpoint.get().read(reinterpret_cast<char*>(chunk_sizes.data()), header.num_of_chunks * sizeof(uint32_t));

    // Calculate total data block size from chunk sizes
    size_t total_size = 0;
    for (auto sz : chunk_sizes) {
        total_size += sizeof(ProjectConfigurations) + sz * sizeof(Task);
    }

    // Read compressed data block
    std::vector<char> raw(total_size);
    endpoint.get().read(raw.data(), total_size);

    // Decompress (passes through for NONE; real algorithm swapped in here later)
    std::vector<char> decompressed;
    decompress_data(static_cast<CompressionType>(header.compression_type), raw.data(), raw.size(), decompressed);

    // Reset container so we load into a clean state
    delete ctx->container;
    ctx->container = new Container();

    size_t offset = 0;
    for (size_t i = 0; i < chunk_sizes.size(); ++i) {
        ProjectConfigurations _tmp_configurations;
        std::memcpy(&_tmp_configurations, decompressed.data() + offset, sizeof(ProjectConfigurations));
        offset += sizeof(ProjectConfigurations);

        ProjConf conf(_tmp_configurations.id, _tmp_configurations.name, _tmp_configurations.description);
        ctx->container->add_project(conf);

        for (uint32_t j = 0; j < chunk_sizes[i]; ++j) {
            TaskConf_t config;
            auto task = std::make_shared<Task>(&config);
            std::memcpy(task.get(), decompressed.data() + offset, sizeof(Task));
            offset += sizeof(Task);
            ctx->container->add_task(i, task.get());
        }
    }

    if (header.user_data_length > 0 && ctx->user_data) {
        memcpy(ctx->user_data, header.user_buffer, header.user_data_length);
    }

    QWISTYS_TELEMETRY_END();
    return Ok();
}

API_SELFMA VoidResult selfma_deserialize(selfma_ctx_t* ctx) {
    if (ctx) {
        return deserialize(hash_to_file(ctx->uuid), ctx);
    }
    return Err(ErrorCode::INPUT, "Null ctx passed to deserialization");
}

API_SELFMA selfma_ctx_t* selfma_create(uint32_t id, const std::string& file_name, char* user_buffer) {
    selfma_ctx_t* ctx = (selfma_ctx_t*) qwistys_malloc(sizeof(selfma_ctx_t), nullptr);

    if (ctx) {
        ctx->container = new Container();
        copy_strchar(file_name, ctx->uuid, MAX_NAME_LENGTH);
        ctx->user_data = user_buffer;
    }
    return ctx;
}

API_SELFMA void selfma_destroy(selfma_ctx_t* ctx) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
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

API_SELFMA void* selfma_get_project(selfma_ctx_t* ctx, uint32_t project_id) {
    if (ctx) {
        return ctx->container->get_project(project_id);
    }
    return nullptr;
}

API_SELFMA void selfma_update(selfma_ctx_t* ctx, void* cbs) {
    if (ctx) {
        QWISTYS_ASSERT(ctx->container);
        ctx->container->update(cbs);
    }
}
