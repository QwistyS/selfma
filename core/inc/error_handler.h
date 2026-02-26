#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "qwistys_macros.h"  // provides QWISTYS_ERROR_MSG

#define PANIC(msg) throw std::runtime_error(msg)

/**
 * @brief selfma-specific error codes.
 */
enum class ErrorCode {
    OK = 0,
    ALLOCATION_FAIL,
    INVALID_ID,
    ID_EXHAUSTED,
    STACK_ERROR,
    STACK_FULL,
    ADD_TASK_FAIL,
    REMOVE_TASK_FAIL,
    ADD_PROJECT_FAIL,
    REMOVE_PROJECT_FAIL,
    PROJECT_NOT_FOUND,
    NO_STORAGE,
    MEMORY_ERROR,
    FILE_NOT_FOUND,
    FILE_OPEN_ERROR,
    WRITE_ERROR,
    READ_ERROR,
    INVALID_FORMAT,
    VERSION_MISMATCH,
    SECURITY_ERROR,
    INPUT,
    MONKEY,
    TOTAL
};

#include "qwistys_error_handler.h"

#endif  // ERROR_HANDLER_H
