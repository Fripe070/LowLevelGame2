#pragma once
#include <string>
#include <GL/glew.h>
#include <SDL_log.h>
#include <spdlog/spdlog.h>

void setupLogging();

#pragma region OpenGL error logging, for use when debug output is not available
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
#pragma endregion

void GLAPIENTRY LogGLCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
);
void LogSDLCallback(void* /*userdata*/, int category, SDL_LogPriority priority, const char *message);


