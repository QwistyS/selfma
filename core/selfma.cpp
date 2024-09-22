#include "selfma.h"
#include <cstdint>
#include "project.h"
#include "qwistys_macros.h"
#include "selfma_api.h"
#include "task.h"

/* Handlers*/

bool Selfma::_handle_mem() {
    // Unrecoverable
    return false;
}

bool _handle_input() {
    return false;
}

bool Selfma::_handle_add_project() {
    // Unrecoverable
    return false;
}

inline VoidResult check_args(const DefaultAPI& args) {
    if ((args.name.size() == 0 || args.description.size() == 0)
        || (args.name.size() >= MAX_NAME_LENGTH || args.description.size() >= MAX_DESCRIPTION_LENGTH)) {
        return Err(ErrorCode::INPUT, "project_add: args sanity fail", Severity::LOW);
    }
    return Ok();
}
/* End of Handlers */

void Selfma::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::MEMORY_ERROR, [this]() { return _handle_mem(); });
    _drp.register_recovery_action(ErrorCode::ADD_PROJECT_FAIL, [this]() { return _handle_add_project(); });
    _drp.register_recovery_action(ErrorCode::INPUT, [this]() { return _handle_input(); });
}

bool Selfma::add_project(const DefaultAPI& args) {
    if (auto ret = check_args(args); ret.is_err()) {
        return _error.handle_error(ret.error());
    }

    if (auto ret = selfma_add_project(_ctx, args.name, args.description); ret.is_err()) {
        return _error.handle_error(ret.error());
    }

    return true;
}

bool Selfma::remove_project(DefaultAPI& args) {
    if (auto ret = check_args(args); ret.is_err()) {
        return _error.handle_error(ret.error());
    }

    if (auto ret = selfma_remove_project(_ctx, args.project_id); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

std::vector<DefaultAPI> Selfma::projects_to_vec() {
    std::vector<DefaultAPI> result;
    result.reserve(10);
    Project* proj = nullptr;
    uint32_t ids = 0;
    while (true) {
        proj = (Project*) selfma_get_project(_ctx, ids++);
        if (!proj) {
            break;
        }

        result.emplace_back(proj->config.name,
                            proj->config.description,
                            proj->config.id,
                        proj->size(),
                    0, 0);
    }
    proj = nullptr;
    QWISTYS_DEBUG_MSG("Size of array %d", result.size());

    return result;
}

bool Selfma::add_task(DefaultAPI& args) {
    if (auto ret = check_args(args); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    if (auto ret = selfma_add_task(_ctx, args.project_id, args.name.c_str(), args.description.c_str(), args.duration);
        ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

bool Selfma::remove_task(DefaultAPI& args) {
    if (auto ret = selfma_remove_task(_ctx, args.project_id, args.task_id); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

bool Selfma::serialize() {
    if (auto ret = selfma_serialize(_ctx); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

bool Selfma::deserialize() {
    if (auto ret = selfma_deserialize(_ctx); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

void Selfma::notify(DefaultAPI& event) {
    if (event.notify < NotifyCode::NOTIFY_TOTAL) {
        return _callbacks[event.notify](&event);
    }
    return;
}

void Selfma::shutdown() {
    selfma_destroy(_ctx);
}

void Selfma::update() {
    (this->*_wrapper)(_ctx);
}

void Selfma::on_update_on(void* p) {
    selfma_update((selfma_ctx_t*) p, &_callbacks);
}

void Selfma::nop_stub(void* p) {
    void(0);
}

void Selfma::evntsystem_off() {
    _wrapper = &Selfma::nop_stub;
}

void Selfma::evntsystem_on() {
    _wrapper = &Selfma::on_update_on;
}

void Selfma::register_callback(NotifyCode notify_id, event_callback cb) {
    _callbacks[notify_id] = std::move(cb);
}
