
#include <cstdint>
#include <cstring>
#include <memory>
#include "container.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "task.h"
#include "selfma_api.h"

typedef struct selfma_opq {
    std::unique_ptr<Container> container;
} selfma_ctx_t;

API_SELFMA selfma_ctx_t* selfma_create(uint32_t id, const char* Name, const char* description) {
    selfma_ctx_t* ctx = (selfma_ctx_t*) qwistys_malloc(sizeof(selfma_ctx_t), nullptr);
    if (ctx) {
        ctx->container = std::make_unique<Container>();
    }
    return ctx;
}

API_SELFMA void selfma_destroy(selfma_ctx_t* ctx) {
    if (ctx) {
        ctx->container.reset();
        qwistys_free(ctx);
    }
}

API_SELFMA bool add_project(selfma_ctx_t* ctx, const char* name, const char* description) {
    if (ctx) {
        ProjConf config(0, name, description);
        ctx->container->add_project(config);
        return true;
    }
    return false;
}

API_SELFMA bool remove_project(selfma_ctx_t* ctx, uint32_t id) {
    if (ctx) {
        ctx->container->remove_project(id);
        return true;
    }
    return false;
}

API_SELFMA bool add_task(selfma_ctx_t* ctx, uint32_t project_id, const char* name, const char* description) {
    if (ctx) {
        QWISTYS_TODO_MSG("Need to make a decision about duration values. >;-()");
        TaskConf_t conf = {0};
        conf.description = (char*)malloc(MAX_DESCRIPTION_LENGTH);
        if (!conf.description) {
            return false;
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
        return true;
    }
    return false;
}

API_SELFMA bool remove_task(selfma_ctx_t* ctx, uint32_t project_id, uint32_t task_id) {
    if (ctx) {
        ctx->container->remove_task(project_id, task_id);
        return true;
    }
    return false;
}

API_SELFMA void selfma_print(selfma_ctx_t* ctx) {
    if (ctx) {
        ctx->container->print();
    };
}
