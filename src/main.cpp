#include "SDL.h"
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <config.hpp>
#include <string>



std::string getShaderInfoLog(GLuint shaderID) {
    GLint infoLogLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::string infoLog(infoLogLength, '\0');
    glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
    return infoLog;
}

std::string getProgramInfoLog(GLuint programID) {
    GLint infoLogLength;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::string infoLog(infoLogLength, '\0');
    glGetProgramInfoLog(programID, infoLogLength, nullptr, infoLog.data());
    return infoLog;
}


bool initGlProgram(const GLuint &programID) {
    GLuint vertShaderID = glCreateShader( GL_VERTEX_SHADER );
    GLuint fragShaderID = glCreateShader( GL_FRAGMENT_SHADER );

    const auto vertShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        void main() {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";
    const auto vertShaderSrcLen = static_cast<GLint>(strlen(vertShaderSrc));
    glShaderSource(vertShaderID, 1, &vertShaderSrc, &vertShaderSrcLen);
    glCompileShader(vertShaderID);

    GLint vertResult = GL_FALSE;
    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &vertResult);
    if (vertResult == GL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vertex shader compilation failed: %s", getShaderInfoLog(vertShaderID).c_str());
        return false;
    }

    auto fragShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;

        void main() {
            FragColor = vec4(1, 1, 1, 1);
        }
    )";
    const auto fragShaderSrcLen = static_cast<GLint>(strlen(vertShaderSrc));
    glShaderSource(fragShaderID, 1, &fragShaderSrc, &fragShaderSrcLen);
    glCompileShader(fragShaderID);

    GLint fragResult = GL_FALSE;
    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &fragResult);
    if (fragResult == GL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fragment shader compilation failed: %s", getShaderInfoLog(fragShaderID).c_str());
        return false;
    }

    GLint progResult = GL_FALSE;
    glAttachShader(programID, vertShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &progResult);
    if (progResult == GL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program linking failed: %s", getProgramInfoLog(programID).c_str());
        return false;
    }

    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    return true;
}


int main(int, char *[])
{
    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }
    // Modern OpenGL with core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        config::window_size[0],
        config::window_size[1],
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create OpenGL context: %s", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, glContext);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize GLEW: %s", glewGetErrorString(glewError));
        return -1;
    }

    GLuint programID = glCreateProgram();
    if (!initGlProgram(programID)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize OpenGL");
        return -1;
    }
    glUseProgram(programID);

    float vertices[] = {
        -1./3, -0.5f, 0.0f,
         1./3, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    GLuint vao;
    glGenVertexArrays(1, &vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

