#ifndef SELFMA_IDS_POOL_H
#define SELFMA_IDS_POOL_H

#include <cstdint>
#include <limits>
#include "error_handler.h"
#include "qwistys_macros.h"
#include "qwistys_stack.h"

class IDs {
public:
    IDs(uint32_t max_ids = std::numeric_limits<uint32_t>::max()) : _next(0), _stack(nullptr), _max_ids(max_ids) {}

    VoidResult init() {
        _next = 0;
        _stack = qwistys_stack_init(sizeof(uint32_t), _max_ids);
        if (_stack) {
            return Ok();
        }
        return Err(ErrorCode::ALLOCATION_FAIL, "Failed to allocate memory for stack", Severity::MEDIUM);
    }

    uint32_t max() { return _max_ids; }

    VoidResult clean() {
        if (_stack) {
            qwistys_stack_free(_stack);
            _stack = nullptr;
            _next = 0;
            QWISTYS_DEBUG_MSG("ID client cleared");
        }
        return Ok();
    }

    VoidResult release(uint32_t id) {
        if (id >= _next) {
            return Err(ErrorCode::INVALID_ID, "Attempt to release invalid ID", Severity::LOW);
        }
        if (qwistys_stack_size(_stack) >= _max_ids) {
            return Err(ErrorCode::STACK_FULL, "Cannot release ID, all IDs are already released", Severity::LOW);
        }
        return qwistys_stack_push(_stack, &id) == 0
                   ? Ok()
                   : Err(ErrorCode::STACK_ERROR, "Failed to push ID to stack", Severity::MEDIUM);
    }

    Result<uint32_t> next() {
        uint32_t ret = 0;
        if (qwistys_stack_is_empty(_stack)) {
            if (_next >= _max_ids) {
                return Err<uint32_t>(ErrorCode::ID_EXHAUSTED, "All IDs have been allocated", Severity::HIGH);
            }
            ret = _next++;
        } else {
            if (qwistys_stack_pop(_stack, &ret) != 0) {
                return Err<uint32_t>(ErrorCode::STACK_ERROR, "Failed to pop ID from stack", Severity::MEDIUM);
            }
        }
        return Ok(ret);
    }

    uint32_t available_ids() const { return _max_ids - (_next - qwistys_stack_size(_stack)); }

private:
    uint32_t _next;
    qwistys_stack_t* _stack;
    uint32_t _max_ids;
};

#endif  // SELFMA_IDS_POOL_H
