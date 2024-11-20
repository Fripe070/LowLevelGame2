#include "logging.h"

#include <unordered_map>

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
        logError("%s:%d OpenGL error: (%d) %s", file, line, errorCode, glErrorString(errorCode).c_str());
    }
    return errorCode;
}

GLenum glLogErrorsExtra_(const char *file, const int line, const char *extra) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        logError("%s:%d OpenGL error %s: (%d) %s", file, line, extra, errorCode, glErrorString(errorCode).c_str());
    }
    return errorCode;
}
GLenum glLogErrorsExtra_(const char *file, const int line, const std::string &extra) {
    return glLogErrorsExtra_(file, line, extra.c_str());
}

void GLAPIENTRY MessageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
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
    std::unordered_map<GLenum, const char*> severityMap = {
        {GL_DEBUG_SEVERITY_HIGH, "High"},
        {GL_DEBUG_SEVERITY_MEDIUM, "Medium"},
        {GL_DEBUG_SEVERITY_LOW, "Low"},
        {GL_DEBUG_SEVERITY_NOTIFICATION, "Notification"}
    };
    std::unordered_map<GLenum, SDL_LogPriority> severitySDLMap = {
        {GL_DEBUG_SEVERITY_HIGH, SDL_LOG_PRIORITY_CRITICAL},  // Real errors or really dangerous undefined behavior
        {GL_DEBUG_SEVERITY_MEDIUM, SDL_LOG_PRIORITY_ERROR},  // Undefined behavior or major performance issues
        {GL_DEBUG_SEVERITY_LOW, SDL_LOG_PRIORITY_WARN},  // Redundant state change or unimportant undefined behavior
        {GL_DEBUG_SEVERITY_NOTIFICATION, SDL_LOG_PRIORITY_VERBOSE}
    };

    const auto err = severitySDLMap.find(severity);
    const auto logPriority = err != severitySDLMap.end() ? err->second : SDL_LOG_PRIORITY_CRITICAL;

#define KEY_OR_UNKNOWN(map, key) (map.find(key) != map.end() ? map[key] : "Unknown")
    logRaw(logPriority,
        "OpenGL %s [%s] (%d) %s %s",
        KEY_OR_UNKNOWN(sourceMap, source),
        KEY_OR_UNKNOWN(typeMap, type),
        id,
        KEY_OR_UNKNOWN(severityMap, severity),
        message
    );
#undef KEY_OR_UNKNOWN
}
