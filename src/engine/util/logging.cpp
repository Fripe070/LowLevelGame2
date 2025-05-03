#include "logging.h"

#include <unordered_map>
#include <spdlog/sinks/stdout_color_sinks-inl.h>

std::string glErrorString(const GLenum errorCode) {
    static const std::unordered_map<GLenum, std::string> map = {
        {GL_NO_ERROR, "No error"},
        {GL_INVALID_ENUM, "Invalid enum"},
        {GL_INVALID_VALUE, "Invalid value"},
        {GL_INVALID_OPERATION, "Invalid operation"},
        {GL_STACK_OVERFLOW, "Stack overflow"},
        {GL_STACK_UNDERFLOW, "Stack underflow"},
        {GL_OUT_OF_MEMORY, "Out of memory"},
        {GL_INVALID_FRAMEBUFFER_OPERATION, "Invalid framebuffer operation"},
        {GL_CONTEXT_LOST, "Context lost"},
        {GL_TABLE_TOO_LARGE, "Table too large"}
    };

    const auto err = map.find(errorCode);
    return err != map.end() ? err->second : "Unknown error: " + std::to_string(errorCode);
}

GLenum glLogErrors_(const char *file, const int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        spdlog::get("opengl")->error("OpenGL error: ({}) {}", errorCode, glErrorString(errorCode));
    }
    return errorCode;
}

GLenum glLogErrorsExtra_(const char *file, const int line, const char *extra) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        spdlog::get("opengl")->error("OpenGL error {}: ({}) {}", extra, errorCode, glErrorString(errorCode));
    }
    return errorCode;
}
GLenum glLogErrorsExtra_(const char *file, const int line, const std::string &extra) {
    return glLogErrorsExtra_(file, line, extra.c_str());
}

void GLAPIENTRY LogGLCallback(
    const GLenum source,
    const GLenum type,
    const GLuint id,
    const GLenum severity,
    const GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    std::unordered_map<GLenum, const char*> sourceMap = {
        {GL_DEBUG_SOURCE_API, "API"},
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window system"},
        {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader compiler"},
        {GL_DEBUG_SOURCE_THIRD_PARTY, "Third party"},
        {GL_DEBUG_SOURCE_APPLICATION, "Application"},
        {GL_DEBUG_SOURCE_OTHER, "Other"}
    };
    std::unordered_map<GLenum, const char*> typeMap = {
        {GL_DEBUG_TYPE_ERROR, "Error"},
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated behavior"},
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined behavior"},
        {GL_DEBUG_TYPE_PORTABILITY, "Non-portable"},
        {GL_DEBUG_TYPE_PERFORMANCE, "Performance"},
        {GL_DEBUG_TYPE_MARKER, "Marker"},
        {GL_DEBUG_TYPE_PUSH_GROUP, "Push group"},
        {GL_DEBUG_TYPE_POP_GROUP, "Pop group"},
        {GL_DEBUG_TYPE_OTHER, "Other"}
    };
    std::unordered_map<GLenum, spdlog::level::level_enum> severityMap = {
        {GL_DEBUG_SEVERITY_HIGH, spdlog::level::level_enum::critical},  // Real errors or really dangerous undefined behavior
        {GL_DEBUG_SEVERITY_MEDIUM, spdlog::level::level_enum::err},  // Undefined behavior or major performance issues
        {GL_DEBUG_SEVERITY_LOW, spdlog::level::level_enum::warn},  // Redundant state change or unimportant undefined behavior
        {GL_DEBUG_SEVERITY_NOTIFICATION, spdlog::level::level_enum::trace}
    };
    const auto errSeverity = severityMap.find(severity);
    const auto logPriority = errSeverity != severityMap.end() ? errSeverity->second : spdlog::level::critical;

#define KEY_OR_UNKNOWN(map, key) (map.find(key) != map.end() ? map[key] : "Unknown")
    spdlog::get("opengl")->log(
        logPriority,
        "{} [{}] ({}) {}",
        KEY_OR_UNKNOWN(sourceMap, source),
        KEY_OR_UNKNOWN(typeMap, type),
        id,
        KEY_OR_UNKNOWN(severityMap, severity)
    );
#undef KEY_OR_UNKNOWN
}

void LogSDLCallback(void*, int category, SDL_LogPriority priority, const char *message) {
    //TODO: Make better lmao
    static const std::unordered_map<SDL_LogPriority, spdlog::level::level_enum> priorityMap = {
        {SDL_LOG_PRIORITY_VERBOSE, spdlog::level::trace},
        {SDL_LOG_PRIORITY_DEBUG, spdlog::level::debug},
        {SDL_LOG_PRIORITY_INFO, spdlog::level::info},
        {SDL_LOG_PRIORITY_WARN, spdlog::level::warn},
        {SDL_LOG_PRIORITY_ERROR, spdlog::level::err},
        {SDL_LOG_PRIORITY_CRITICAL, spdlog::level::critical}
    };
    const auto err = priorityMap.find(priority);
    const auto logLevel = err != priorityMap.end() ? err->second : spdlog::level::critical;
    spdlog::get("opengl")->log(logLevel, "{}", message);
}

void setupLogging() {
    const auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);

    const auto openglLogger = spdlog::stdout_color_mt("opengl");
    openglLogger->set_level(spdlog::level::debug);
}
