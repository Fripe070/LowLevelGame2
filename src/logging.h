#ifndef LOGGING_H
#define LOGGING_H

#include <unordered_map>


#define logError(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

inline std::string glErrorString(const GLenum errorCode) {
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

inline GLenum glLogErrors_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        logError( "OpenGL error: (%d) %s at %s:%d", errorCode, glErrorString(errorCode).c_str(), file, line);
    }
    return errorCode;
}

#ifdef DEBUG
#define glLogErrors() glLogErrors_(__FILE__, __LINE__)  // glGetError is a bit slow
#else
#define glLogErrors()
#endif


#endif //LOGGING_H
