#include "selfma.h"
#include <features.h>
#include <system_error>
#include "error_handler.h"
#include "project.h"
#include "selfma_api.h"
#include "task.h"

/* Handlers*/

bool Selfma::_handle_mem() {
    // Unrecoverable
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
}

bool Selfma::project_add(DefaultAPI& args) {
    
    if ((args.name.size() == 0 || args.descritpion.size() == 0) || (args.name.size() >= MAX_NAME_LENGTH|| args.descritpion.size() >= MAX_DESCRIPTION_LENGTH)) {
        return _error.handle_error(Err(ErrorCode::INPUT, "project_add: args sanity fail", Severity::LOW).error());
    }

    if (auto ret = selfma_add_project(_ctx, args.name.c_str(), args.descritpion.c_str()); ret.is_err()) {
        return _error.handle_error(ret.error());
    }

    return true;
}

bool Selfma::project_remove(DefaultAPI& args) {
    return selfma_remove_project(_ctx, args.project_id).is_err();    
}


bool Selfma::project_add_task(DefaultAPI& args) {
    if ((args.name.size() == 0 || args.descritpion.size() == 0) || (args.name.size() >= MAX_NAME_LENGTH|| args.descritpion.size() >= MAX_DESCRIPTION_LENGTH)) {
        return _error.handle_error(Err(ErrorCode::INPUT, "project_add_task: args sanity fail", Severity::LOW).error());
    }
    if (auto ret = selfma_add_task(_ctx, args.project_id, args.name.c_str(), args.descritpion.c_str()); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

bool Selfma::project_remove_task(DefaultAPI& args) {
    if (auto ret = selfma_remove_task(_ctx, args.project_id, args.task_id); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}
