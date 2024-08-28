#include "selfma.h"
#include "error_handler.h"
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

/* End of Handlers */

void Selfma::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::MEMORY_ERROR, [this]() { return _handle_mem(); });
    _drp.register_recovery_action(ErrorCode::ADD_PROJECT_FAIL, [this]() { return _handle_add_project(); });
    _drp.register_recovery_action(ErrorCode::INPUT, [this]() { return _handle_input(); });
}

bool Selfma::add_project(DefaultAPI& args) {
    if ((args.name.size() == 0 || args.descritpion.size() == 0)
        || (args.name.size() >= MAX_NAME_LENGTH || args.descritpion.size() >= MAX_DESCRIPTION_LENGTH)) {
        return _error.handle_error(Err(ErrorCode::INPUT, "project_add: args sanity fail", Severity::LOW).error());
    }

    if (auto ret = selfma_add_project(_ctx, args.name.c_str(), args.descritpion.c_str()); ret.is_err()) {
        return _error.handle_error(ret.error());
    }

    return true;
}

bool Selfma::remove_project(DefaultAPI& args) {
    if (auto ret = selfma_remove_project(_ctx, args.project_id); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

bool Selfma::add_task(DefaultAPI& args) {
    if ((args.name.size() == 0 || args.descritpion.size() == 0)
        || (args.name.size() >= MAX_NAME_LENGTH || args.descritpion.size() >= MAX_DESCRIPTION_LENGTH)) {
        return _error.handle_error(Err(ErrorCode::INPUT, "project_add_task: args sanity fail", Severity::LOW).error());
    }
    if (auto ret = selfma_add_task(_ctx, args.project_id, args.name.c_str(), args.descritpion.c_str(), args.duration); ret.is_err()) {
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

void Selfma::notify(DefaultAPI& event) {
    if (event.notify < NotifyCode::NOTIFY_TOTAL) {
        return _callbacks[event.notify](&event);
    }
    return;
}

void Selfma::update() {
    QWISTYS_TODO_MSG("Who is responsable to clear interupt ?");
    selfma_update(_ctx);
}
