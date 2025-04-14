#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <spdlog/spdlog.h>


struct ErrorPos {
    std::string file;
    int line;
};
struct Error {
    std::string message;
    std::optional<ErrorPos> pos;
    std::shared_ptr<Error> cause;
};

template<typename T>
using Expected = std::expected<T, Error>;

/*!
 * Creates a new error with a message.
 * @param message The error message.
 */
#define ERROR(message) Error{message, ErrorPos{__FILE__, __LINE__}, nullptr}
/*!
 * Creates a new error with a referenced cause.
 * @param error The error that caused this error.
 * @param message The error message.
 */
#define FW_ERROR(error, message) Error{message, ErrorPos{__FILE__, __LINE__}, std::make_shared<Error>(error)}

/*! Transforms an error into a nicely formatted multiline string. */
inline std::string stringifyError(Error error) {
    std::string message = error.message;
    while (error.cause) {
        error = *error.cause;
        message += "\n  ";
        if (error.pos.has_value() && !error.message.empty())
            message += "at " + error.pos->file + ":" + std::to_string(error.pos->line) + ": " + error.message;
        else if (error.pos.has_value() && error.message.empty())
            message += "at " + error.pos->file + ":" + std::to_string(error.pos->line) + "";
        else if (!error.pos.has_value() && !error.message.empty())
            message += error.message;
    }
    return message;
}
/*! Logs an error to the console using spdlog with error severity. */
inline void reportError(const Error& error) {
    SPDLOG_ERROR(stringifyError(error));
}
