#ifndef SELFMA_SELFMA_H
#define SELFMA_SELFMA_H

#include <cstdint>
#include "error_handler.h"
#include "selfma_api.h"

// Like a command thing. can be staticly allocated for single reff, 
// on each req pass this structure
struct DefaultAPI {
  std::string name;
  std::string descritpion;
  uint32_t project_id;
  uint32_t task_id;
    
};

// Core Lib wrapper to c++
class Selfma final {
public:
    Selfma(const char* hash_id, const char* buffer) : _error(_drp) {
        _ctx = selfma_create(0, hash_id, buffer);
        if (!_ctx) {
            auto e = Error(ErrorCode::MEMORY_ERROR, "Fail to create context", Severity::CRITICAL);
            _error.handle_error(e);
        }
    };

    ~Selfma() {
        selfma_destroy(_ctx);
    };

    // Do i need project/task update ? or just rewrite existing?
    bool project_add(DefaultAPI& args);
    bool project_remove(DefaultAPI& args);
    bool project_add_task(DefaultAPI& args);
    bool project_remove_task(DefaultAPI& args);
    bool project_serialize();

private:
    selfma_ctx_t *_ctx;
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    void _setup_drp();
    bool _handle_mem();
    bool _handle_add_project();
};

#endif  // SELFMA_SELFMA_H
