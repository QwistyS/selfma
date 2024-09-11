#ifndef SELFMA_SELFMA_H
#define SELFMA_SELFMA_H

#include "task.h"
#include "error_handler.h"
#include "selfma_api.h"

// Core Lib wrapper to c++
class Selfma final {
public:
    Selfma(const std::string& file_name, char* buffer) : _error(_drp), _wrapper(&Selfma::nop_stub) {
        _ctx = selfma_create(0, file_name, buffer);
        if (!_ctx) {
            auto e = Error(ErrorCode::MEMORY_ERROR, "Fail to create context", Severity::CRITICAL);
            _error.handle_error(e);
        }
    };

    ~Selfma() { selfma_destroy(_ctx); };

    static void wrapper_on_update_on(void* p) {
        Selfma* self = static_cast<Selfma*>(p);
        self->on_update_on(p);
    }
    
    // Do i need project/task update ? or just rewrite existing?
    bool add_project(const DefaultAPI& args);
    bool add_task(DefaultAPI& args);
    bool remove_project(DefaultAPI& args);
    bool remove_task(DefaultAPI& args);
    void register_callback(NotifyCode notify_id, event_callback cb);
    bool serialize();
    bool deserialize();
    void notify(DefaultAPI& event);
    void update();
    void evntsystem_on();
    void evntsystem_off();
    void shutdown();

private:
    selfma_ctx_t* _ctx;
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    
    std::array<event_callback, NotifyCode::NOTIFY_TOTAL> _callbacks;
    void _setup_drp();
    bool _handle_mem();
    bool _handle_add_project();
    void on_update_on(void* p);
    void on_update_off(void* p);
    void nop_stub(void* p);
    typedef void (Selfma::*WrapperFunc)(void*);
    WrapperFunc _wrapper;

};

#endif  // SELFMA_SELFMA_H
