#ifndef SELFMA_SELFMA_H
#define SELFMA_SELFMA_H

#include <array>
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
    NOTIFY_OK = 0,
    EVENT_MAX_TIME_SLEEP,
    NOTIFY_TOTAL,
};

typedef void (*event_callback)(DefaultAPI*);

// Core Lib wrapper to c++
class Selfma final {
public:
    Selfma(const std::string& file_name, char* buffer) : _error(_drp) {
        _ctx = selfma_create(0, file_name, buffer);
        if (!_ctx) {
            auto e = Error(ErrorCode::MEMORY_ERROR, "Fail to create context", Severity::CRITICAL);
            _error.handle_error(e);
        }
    };

    ~Selfma() { selfma_destroy(_ctx); };

    // Do i need project/task update ? or just rewrite existing?
    bool add_project(DefaultAPI& args);
    bool add_task(DefaultAPI& args);
    bool remove_project(DefaultAPI& args);
    bool remove_task(DefaultAPI& args);
    void register_callback(uint32_t notify_id, event_callback cb) { _callbacks[notify_id] = std::move(cb); }
    bool serialize();
    void notify(DefaultAPI& event);
    void update();
    void shutdown();

private:
    selfma_ctx_t* _ctx;
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    std::array<event_callback, NotifyCode::NOTIFY_TOTAL> _callbacks;
    void _setup_drp();
    bool _handle_mem();
    bool _handle_add_project();
};

#endif  // SELFMA_SELFMA_H
