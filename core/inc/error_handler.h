#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "qwistys_macros.h"
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#define PANIC(msg) throw std::runtime_error(msg)

/**
 * @brief Global error code for 5thd application may be splitted by context in future
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
/**
 * @brief Error severity for disaster recover stuff
 */
enum class Severity { LOW, MEDIUM, HIGH, CRITICAL };

/**
 * @brief Error object holds the error information
 */
class Error {
public:
    Error(ErrorCode code, const std::string& message, Severity severity)
        : code_(code), message_(message), severity_(severity) {}

    ErrorCode code() const { return code_; }
    const std::string& message() const { return message_; }
    Severity severity() const { return severity_; }

private:
    ErrorCode code_;
    std::string message_;
    Severity severity_;
};

/**
 * @brief Result is a abstraction for handling errors wrapper around of actual result from function
 * @note Used as type, a bit cost performance but worth it.
 */
template <typename T>
class Result {
public:
    explicit Result(T value) : value_(std::move(value)) {}
    explicit Result(Error error) : _error(std::move(error)) {}
    bool is_ok() const { return !_error.has_value(); }
    bool is_err() const { return _error.has_value(); }
    const T& value() const {
        if (_error)
            throw std::runtime_error("Result contains an error");
        return *value_;
    }
    const Error& error() const {
        if (!_error)
            throw std::runtime_error("Result does not contain an error");
        return *_error;
    }

private:
    std::optional<T> value_;
    std::optional<Error> _error;
};

template <typename T>
inline Result<T> Ok(T value) {
    return Result<T>(std::move(value));
};

template <typename T>
inline Result<T> Err(ErrorCode code, const std::string& message, Severity severity = Severity::MEDIUM) {
    return Result<T>(Error(code, message, severity));
};

/**
 * @brief Same as Result class only for void functions
 * @note User as return type of the function/method
 */
class VoidResult {
public:
    explicit VoidResult() = default;                                // Default constructor for success
    explicit VoidResult(Error error) : _error(std::move(error)) {}  // Constructor for error

    bool is_ok() const { return !_error.has_value(); }
    bool is_err() const { return _error.has_value(); }
    const Error& error() const {
        if (!_error)
            throw std::runtime_error("VoidResult does not contain an error");
        return *_error;
    }

private:
    std::optional<Error> _error;
};

inline VoidResult Ok() {
    return VoidResult();
}

inline VoidResult Err(ErrorCode code, const std::string& message, Severity severity = Severity::MEDIUM) {
    return VoidResult(Error(code, message, severity));
}

/**
 * @brief Class will hold DRP callbacks for specific error
 * @note in case of fail will throw std::runtime_error();
 */
class DisasterRecoveryPlan {
public:
    /**
     * @brief c++ equivalent of c typedef for callback bool(recovery_action*)()
     */
    using RecoveryAction = std::function<bool()>;
    /**
     * @brief Register the callback on error
     */
    void register_recovery_action(ErrorCode code, RecoveryAction action) {
        _recovery_actions[code] = std::move(action);
    }
    /**
     * @brief Actual recovery call
     * @return bool
     */
    bool execute_recovery(const Error& error) {
        if (auto it = _recovery_actions.find(error.code()); it != _recovery_actions.end()) {
            // Holds actual callback
            return it->second();
        }
        return false;
    }

private:
    std::unordered_map<ErrorCode, RecoveryAction> _recovery_actions;
};

/**
 * @brief Error handler class handles automaticaly
 */
class ErrorHandler {
public:
    /**
     * @brief c++ equivalent of c typedef for callback void(const Error*)()
     */
    using ErrorCallback = std::function<void(const Error&)>;

    explicit ErrorHandler(DisasterRecoveryPlan& drp) : drp_(drp) {}

    /**
     * @brief Register a callback on specific erro
     * @param ErrorCode the error code
     * @paragraph ErrorCallback the function you wanna call on it
     */
    void register_callback(ErrorCode code, ErrorCallback callback) { callbacks_[code] = std::move(callback); }

    /**
     * @brief Handles error.
     * @note Will call DRP action if registered on particular error with level >= HIGH.
     * @note No need to log error will do automatically.
     * @note Also can be added independet callback on particular error.
     * @return bool in case of DRP HIGH or above fail will throw runtime_error & abort.
     */
    bool handle_error(const Error& error) const {
        if (auto it = callbacks_.find(error.code()); it != callbacks_.end()) {
            it->second(error);
        }

        bool recovered = false;
        if (error.severity() >= Severity::HIGH) {
            recovered = drp_.execute_recovery(error);
            if (!recovered && error.severity() == Severity::CRITICAL) {
                QWISTYS_ERROR_MSG("BAD ERROR: {}", error.message().c_str());
                std::abort();
            }
        }

        log_error(error);
        return recovered;
    }

private:
    std::unordered_map<ErrorCode, ErrorCallback> callbacks_;
    DisasterRecoveryPlan& drp_;

    void log_error(const Error& error) const {
        QWISTYS_ERROR_MSG("[ Severity: %d Error code: %d Error message: %s ]", static_cast<int>(error.severity()),
              static_cast<int>(error.code()), error.message().c_str());
    }
};

#endif  // ERROR_HANDLER_H
