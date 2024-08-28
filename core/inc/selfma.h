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
  double duration;
  uint32_t notify;
};


enum NotifyCode {
    OK = 0,
    MAX_SLEEP_TIME,
    TOTAL
};


// Core Lib wrapper to c++
class Selfma final {
public:
   using Notify = std::function<void(const DefaultAPI&)>;


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
    bool add_project(DefaultAPI& args);
    bool add_task(DefaultAPI& args);
    bool remove_project(DefaultAPI& args);
    bool remove_task(DefaultAPI& args);
    void register_callback(uint32_t notify_id, Notify callback) { _callbacks[notify_id] = std::move(callback); }
    bool serialise();
    void notify(DefaultAPI& event);

private:
    selfma_ctx_t *_ctx;
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    std::unordered_map<uint32_t, Notify> _callbacks;
    void _setup_drp();
    bool _handle_mem();
    bool _handle_add_project();
};

#endif  // SELFMA_SELFMA_H
