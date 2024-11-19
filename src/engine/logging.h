#ifndef LOGGING_H
#define LOGGING_H
#include <string>
#include <gl/glew.h>
// ReSharper disable once CppUnusedIncludeDirective // Used in macros
#include <SDL_log.h>

std::string glErrorString(GLenum errorCode);

GLenum glLogErrors_(const char *file, int line);
GLenum glLogErrorsExtra_(const char *file, int line, const char *extra);
GLenum glLogErrorsExtra_(const char *file, int line, const std::string &extra);

#ifndef NDEBUG
#define glLogErrors() glLogErrors_(__FILE__, __LINE__)  // glGetError is a bit slow
#define glLogErrorsExtra(extra) glLogErrorsExtra_(__FILE__, __LINE__, extra)
#else
#define glLogErrors()
#define glLogErrorsExtra(extra)
#endif

#define FILE_REF (__FILE__ ":" + std::to_string(__LINE__))

#define logRaw(...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define logSeverity(severity, fmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, severity, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)
#define logError(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)
#define logWarn(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)
#define logInfo(fmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)
#define logDebug(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)
#define logVerbose(fmt, ...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + " " + fmt).c_str(), ##__VA_ARGS__)

void GLAPIENTRY MessageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
);

#endif //LOGGING_H
