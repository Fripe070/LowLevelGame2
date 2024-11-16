#ifndef LOGGING_H
#define LOGGING_H
#include <string>
#include <gl/glew.h>
// ReSharper disable once CppUnusedIncludeDirective // Used in macros
#include <SDL_log.h>

std::string glErrorString(GLenum errorCode);

GLenum glLogErrors_(const char *file, int line);

#define logError(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, \
    (__FILE__ ":" + std::to_string(__LINE__) + " " + fmt).c_str(), ##__VA_ARGS__)
#define logWarn(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, \
    (__FILE__ ":" + std::to_string(__LINE__) + " " + fmt).c_str(), ##__VA_ARGS__)
#define logInfo(fmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, \
    (__FILE__ ":" + std::to_string(__LINE__) + " " + fmt).c_str(), ##__VA_ARGS__)
#define logDebug(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, \
    (__FILE__ ":" + std::to_string(__LINE__) + " " + fmt).c_str(), ##__VA_ARGS__)

#ifndef NDEBUG
#define glLogErrors() glLogErrors_(__FILE__, __LINE__)  // glGetError is a bit slow
#else
#define glLogErrors()
#endif


#endif //LOGGING_H
