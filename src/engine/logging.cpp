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