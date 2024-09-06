#include <string>
#include <gl/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui_impl_sdl2.h>
#include "shader.h"
#include "gui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define DEBUG

ProgState progState;

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
        uv,
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

    constexpr float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
       -0.5f, -0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
       -0.5f,  0.5f,  0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f // top left
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
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

    const unsigned int fstride = 8 * sizeof(float);
    glVertexAttribPointer(
        t_attribute_ids::position, 3, GL_FLOAT, GL_FALSE,
        fstride, nullptr);
    glEnableVertexAttribArray(t_attribute_ids::position);
    glVertexAttribPointer(
        t_attribute_ids::colour, 3, GL_FLOAT, GL_FALSE,
        fstride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::colour);
    glVertexAttribPointer(
        t_attribute_ids::uv, 2, GL_FLOAT, GL_FALSE,
        fstride, reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::uv);

    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

    GLuint texture;
    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* imgData = stbi_load("resources/brick.png", &width, &height, &nrChannels, 0);
    if (!imgData) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture %s: %s", "resources/container.png", stbi_failure_reason());
        return -1;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    // If consulting this code in regard to output being messed up, make sure to use alpha channel if present
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(imgData);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const auto shader = Shader("resources/vert.glsl", "resources/frag.glsl");

    auto trans = glm::mat4(1.0f);
    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");

    GUI::init(window, glContext);

    bool running = true;
    Uint64 frameStart = SDL_GetPerformanceCounter();
    SDL_Event event;
    while (running) {
        const Uint64 lastFrameStart = frameStart;
        frameStart = SDL_GetPerformanceCounter();
        const double deltaTime = (frameStart - lastFrameStart) / static_cast<double>(SDL_GetPerformanceFrequency());
        const double expectedDT = 1.0 / progState.maxFPS;
        if (progState.limitFPS && !progState.vsync && deltaTime < expectedDT) {
            SDL_Delay((expectedDT - deltaTime) * 1000);
        }

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                running = false;
                break;
            }
        }
        if (!running)
            break;

        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        trans = glm::mat4(1.0f); // remove to make the rotation angles to spin speed

        // TODO : use a single rotation matrix
        trans = glm::rotate(trans, glm::radians(progState.rotX), glm::vec3(1.0, 0.0, 0.0)); // x
        trans = glm::rotate(trans, glm::radians(progState.rotY), glm::vec3(0.0, 1.0, 0.0)); // y
        trans = glm::rotate(trans, glm::radians(progState.rotZ), glm::vec3(0.0, 0.0, 1.0)); // z
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

#ifdef DEBUG
        if (GLenum error = glGetError(); error != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %d", error);
        }
#endif

        GUI::frame(progState);
        ImGui::SetNextWindowPos(ImVec2(5, 5));
        ImGui::SetNextWindowSize(ImVec2(90, 0));
        ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("FPS: %.1f", 1.0 / deltaTime);
        ImGui::End();

        GUI::render();
        SDL_GL_SwapWindow(window);
    }

    GUI::shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
