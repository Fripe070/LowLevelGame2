#include "SDL.h"
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <Shader.h>
#include <string>

namespace config {
    inline int window_size[2] = {1920/2, 1080/2};
}

std::string getShaderInfoLog(const GLuint shaderID) {
    GLint infoLogLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::string infoLog(infoLogLength, '\0');
    glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
    return infoLog;
}

std::string getProgramInfoLog(const GLuint programID) {
    GLint infoLogLength;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::string infoLog(infoLogLength, '\0');
    glGetProgramInfoLog(programID, infoLogLength, nullptr, infoLog.data());
    return infoLog;
}

typedef struct {
    enum {
        position,
        colour,
    };
} t_attribute_ids;


int main(int, char *[])
{
    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        config::window_size[0], config::window_size[1],
        SDL_WINDOW_OPENGL //| SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Modern OpenGL with core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(window, glContext);

    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize GLEW: %s", glewGetErrorString(glewError));
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // const GLuint programID = glCreateProgram();
    // if (!initGlProgram(programID)) {
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize OpenGL");
    //     SDL_GL_DeleteContext(glContext);
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return -1;
    // }

    constexpr float vertices[] = {
         // Positions          // Colour
         0.0f,   0.5f, 0.0f,    1.0f, 0.0f, 0.0f,
         1.f/3, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,
        -1.f/3, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao); // Bind our VAO, essentially our context/config

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(
        t_attribute_ids::position, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(t_attribute_ids::position);
    glVertexAttribPointer(
        t_attribute_ids::colour, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::colour);

    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const auto shader = Shader("resources/vert.glsl", "resources/frag.glsl");

    while (true) {
        SDL_PumpEvents(); // we might want to use SDL_PollEvent later
        if (SDL_QuitRequested())
            break;

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader.programID);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

