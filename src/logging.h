#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <gl/glew.h>

std::string glErrorString(GLenum errorCode);
GLenum glLogErrors_(const char *file, int line);

#define logError(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#ifdef DEBUG
#define glLogErrors() glLogErrors_(__FILE__, __LINE__)  // glGetError is a bit slow
#else
#define glLogErrors()
#endif


#endif //LOGGING_H
