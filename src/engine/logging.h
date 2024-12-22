#ifndef LOGGING_H
#define LOGGING_H
#include <string>
#include <gl/glew.h>
// ReSharper disable once CppUnusedIncludeDirective // Used in macros
#include <SDL_log.h>
#include <spdlog/spdlog.h>

void setupLogging();


#pragma region OpenGL error logging, for use when debug output is not available (looking at you, macOS)
std::string glErrorString(GLenum errorCode);
GLenum glLogErrors_(const char *file, int line);
GLenum glLogErrorsExtra_(const char *file, int line, const char *extra);
GLenum glLogErrorsExtra_(const char *file, int line, const std::string &extra);
#ifndef NDEBUG  // glGetError is a bit slow, so only use it in debug builds
#define glLogErrors() glLogErrors_(__FILE__, __LINE__)
#define glLogErrorsExtra(extra) glLogErrorsExtra_(__FILE__, __LINE__, extra)
#else
#define glLogErrors()
#define glLogErrorsExtra(extra)
#endif
#pragma endregion

#pragma region Helper macros for dealing with and propogating std::unexpected
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define INDENT4 "    "
#define NL_INDENT "\n" INDENT4

#define FILE_REF std::string(__FILE__ ":" STRINGIFY(__LINE__) " ")
#define FW_UNEXP(unexpected, thisTime) (FILE_REF + thisTime + NL_INDENT + unexpected.error())

#define UNEXPECTED_REF(...) std::unexpected(FILE_REF + __VA_ARGS__)
#pragma endregion

// TODO: Use spdlog directly instead of routing through SDL_Log
#define logRaw(...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define logSeverity(severity, fmt, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, severity, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)
#define logError(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)
#define logWarn(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)
#define logInfo(fmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)
#define logDebug(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)
#define logVerbose(fmt, ...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, \
    (FILE_REF + fmt).c_str(), ##__VA_ARGS__)

// Reroute OpenGL and SDL log messages to our own log functions
void GLAPIENTRY LogGlCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
);
void LogSdlCallback(void* /*userdata*/, int category, SDL_LogPriority priority, const char *message);


#endif //LOGGING_H
