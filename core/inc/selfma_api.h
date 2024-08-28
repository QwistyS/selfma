#ifndef SELFMA_API_H
#define SELFMA_API_H

#include "error_handler.h"
#include "stdint.h"

typedef struct selfma_opq selfma_ctx_t;

#define HEADER_VERSION 1
#define API_SELFMA

// Core API
API_SELFMA selfma_ctx_t* selfma_create(uint32_t id, const char* container_id, const char* user_buffer);
API_SELFMA void selfma_destroy(selfma_ctx_t* ctx);
API_SELFMA VoidResult selfma_add_project(selfma_ctx_t* ctx, const char* name, const char* description);
API_SELFMA VoidResult selfma_remove_project(selfma_ctx_t* ctx, uint32_t id);
API_SELFMA VoidResult selfma_add_task(selfma_ctx_t* ctx, uint32_t project_id, const char* name, const char* description, uint32_t duration);
API_SELFMA VoidResult selfma_remove_task(selfma_ctx_t* ctx, uint32_t project_id, uint32_t task_id);
API_SELFMA VoidResult selfma_serialize(selfma_ctx_t* ctx);
API_SELFMA VoidResult selfma_deserialize(selfma_ctx_t* ctx);
API_SELFMA void selfma_update(selfma_ctx_t* ctx);

//  Optionall, created, not connected yet.
API_SELFMA void selfma_print(selfma_ctx_t* ctx);


#endif  // SELFMA_API_H
