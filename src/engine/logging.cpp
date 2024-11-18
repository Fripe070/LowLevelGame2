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
    std::unordered_map<GLenum, std::string> sourceMap = {
        {GL_DEBUG_SOURCE_API, "API"},
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window system"},
        {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader compiler"},
        {GL_DEBUG_SOURCE_THIRD_PARTY, "Third party"},
        {GL_DEBUG_SOURCE_APPLICATION, "Application"},
        {GL_DEBUG_SOURCE_OTHER, "Other"}
    };
    std::unordered_map<GLenum, std::string> typeMap = {
        {GL_DEBUG_TYPE_ERROR, "Error"},
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated behavior"},
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined behavior"},
        {GL_DEBUG_TYPE_PORTABILITY, "Portability"},
        {GL_DEBUG_TYPE_PERFORMANCE, "Performance"},
        {GL_DEBUG_TYPE_MARKER, "Marker"},
        {GL_DEBUG_TYPE_PUSH_GROUP, "Push group"},
        {GL_DEBUG_TYPE_POP_GROUP, "Pop group"},
        {GL_DEBUG_TYPE_OTHER, "Other"}
    };
    std::unordered_map<GLenum, std::string> severityMap = {
        {GL_DEBUG_SEVERITY_HIGH, "High"},
        {GL_DEBUG_SEVERITY_MEDIUM, "Medium"},
        {GL_DEBUG_SEVERITY_LOW, "Low"},
        {GL_DEBUG_SEVERITY_NOTIFICATION, "Notification"}
    };
    std::unordered_map<GLenum, SDL_LogPriority> severitySDLMap = {
        {GL_DEBUG_SEVERITY_HIGH, SDL_LOG_PRIORITY_CRITICAL},
        {GL_DEBUG_SEVERITY_MEDIUM, SDL_LOG_PRIORITY_ERROR},
        {GL_DEBUG_SEVERITY_LOW, SDL_LOG_PRIORITY_WARN},
        {GL_DEBUG_SEVERITY_NOTIFICATION, SDL_LOG_PRIORITY_INFO}
    };

    std::string sourceStr = sourceMap[source];
    if (sourceStr.empty())
        sourceStr = "Unknown";
    std::string typeStr = typeMap[type];
    if (typeStr.empty())
        typeStr = "Unknown";
    std::string severityStr = severityMap[severity];
    if (severityStr.empty())
        severityStr = "Unknown";
    const std::string idStr = glErrorString(id);

    logSeverity(severitySDLMap[severity],
        "OpenGL %s [%s] (%d) %s %s",
        sourceStr.c_str(), typeStr.c_str(), severityStr.c_str(), idStr.c_str(), message);
}
