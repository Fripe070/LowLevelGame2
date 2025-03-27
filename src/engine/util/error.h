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

#define ERROR(message) Error{message, ErrorPos{__FILE__, __LINE__}, nullptr}
#define FW_ERROR(error, message) Error{message, ErrorPos{__FILE__, __LINE__}, std::make_shared<Error>(error)}

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
inline void reportError(const Error& error) {
    SPDLOG_ERROR(stringifyError(error));
}
