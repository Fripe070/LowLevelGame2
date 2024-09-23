#include <string>
#include <unordered_map>
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

    auto err = map.find(errorCode);
    return err!= map.end() ? err->second : "Unknown error: " + std::to_string(errorCode);
}

GLuint loadTexture(const char* filePath) {
    GLuint texture;
    glGenTextures(1, &texture);

    int width, height, channelCount;
    stbi_uc* imgData = stbi_load(filePath, &width, &height, &channelCount, 0);
    if (!imgData) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture \"%s\": %s", filePath, stbi_failure_reason());
        return -1;
    }
    GLint format;
    switch (channelCount) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default: format = GL_RGB;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(imgData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

struct {
    float lastX = static_cast<float>(progState.windowWidth) / 2.0f;
    float lastY = static_cast<float>(progState.windowHeight) / 2.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
} mouseState;

void handle_mouse_movement(const SDL_Event &event, glm::vec3 &cameraFront)
{
    auto xOffset = static_cast<float>(event.motion.xrel);
    auto yOffset = static_cast<float>(-event.motion.yrel);
    xOffset *= progState.sensitivity;
    yOffset *= progState.sensitivity;

    mouseState.yaw += xOffset;
    mouseState.pitch += yOffset;
    mouseState.pitch = std::max(-89.0f, std::min(mouseState.pitch, 89.0f));

    const float pitch = glm::radians(mouseState.pitch);
    const float yaw = glm::radians(mouseState.yaw);
    cameraFront = glm::vec3(
        cos(yaw) * cos(pitch),
        sin(pitch),
        sin(yaw) * cos(pitch)
    );
}

void handle_scroll(double yOffset)
{
    progState.fov -= static_cast<float>(yOffset);
    progState.fov = std::max(progState.fov, 1.0f);
    progState.fov = std::min(progState.fov, 45.0f);
    // for some reason, while using clamp it crashes on startup
    // progState.fov = std::clamp(progState.fov, 1.0f, 45.0f);
}

typedef struct {
    enum {
        position,
        normal,
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
        progState.windowWidth, progState.windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
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
    SDL_SetRelativeMouseMode(SDL_TRUE);

    constexpr float vertices[] = {
        // positions           // Normals          // colors           // texture coords
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
    };
    // constexpr unsigned int indices[] = {
    //     0, 1, 2,
    //     2, 3, 0
    // };
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    GLuint vao, vbo;//, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // glGenBuffers(1, &ebo);

    glBindVertexArray(vao); // Bind our VAO, essentially our context/config

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    constexpr unsigned int valueCount = 3*3 + 2;
    constexpr unsigned int fStride = valueCount * sizeof(float);
    glVertexAttribPointer(
        t_attribute_ids::position, 3, GL_FLOAT, GL_FALSE,
        fStride, nullptr);
    glEnableVertexAttribArray(t_attribute_ids::position);
    glVertexAttribPointer(
        t_attribute_ids::normal, 3, GL_FLOAT, GL_FALSE,
        fStride, reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::normal);
    glVertexAttribPointer(
        t_attribute_ids::colour, 3, GL_FLOAT, GL_FALSE,
        fStride, reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::colour);
    glVertexAttribPointer(
        t_attribute_ids::uv, 2, GL_FLOAT, GL_FALSE,
        fStride, reinterpret_cast<void*>(9 * sizeof(float)));
    glEnableVertexAttribArray(t_attribute_ids::uv);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to _bind_ the VBO, it already contains the data we need
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, fStride, nullptr);
    glEnableVertexAttribArray(0);


    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint diffuseTex = loadTexture("resources/brick.png");
    GLuint specularTex = loadTexture("resources/brick_specular.png");

    const auto shader = Shader("resources/vert.vert", "resources/frag.frag");
    const auto lightShader = Shader("resources/vert.vert", "resources/light.frag");

    glEnable(GL_DEPTH_TEST);
    GUI::init(window, glContext);

    glm::mat4 viewMatrix;
    // TODO: Break camera into its own class
    auto cameraPos = glm::vec3(0.0f, 0.0f,  3.0f);
    auto cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    auto cameraUp = glm::vec3(0.0f, 1.0f,  0.0f);

    auto lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

    Uint64 frameStart = SDL_GetPerformanceCounter();
    SDL_Event event;
    while (true) {
        const Uint64 lastFrameStart = frameStart;
        frameStart = SDL_GetPerformanceCounter();
        const double deltaTime = static_cast<double>(frameStart - lastFrameStart) / static_cast<double>(SDL_GetPerformanceFrequency());
        const auto fDeltaTime = static_cast<float>(deltaTime); // We need it as a float a lot

        // TODO: Process inputs instead of sleeping, to mitigate some input lag
        if (const double expectedDT = 1.0 / progState.maxFPS;
            progState.limitFPS && !progState.vsync && deltaTime < expectedDT) {
            SDL_Delay(static_cast<Uint32>((expectedDT - deltaTime) * 1000.0));
        }

        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        auto inputDir = glm::vec3(0.0f, 0.0f,  0.0f);
        if (keyState[SDL_SCANCODE_W])
            inputDir += cameraFront;
        if (keyState[SDL_SCANCODE_S])
            inputDir -= cameraFront;
        if (keyState[SDL_SCANCODE_A])
            inputDir -= glm::normalize(glm::cross(cameraFront, cameraUp));
        if (keyState[SDL_SCANCODE_D])
            inputDir += glm::normalize(glm::cross(cameraFront, cameraUp));
        if (keyState[SDL_SCANCODE_SPACE])
            inputDir += cameraUp;
        if (keyState[SDL_SCANCODE_LSHIFT])
            inputDir -= cameraUp;
        if (keyState[SDL_SCANCODE_ESCAPE])
            SDL_SetRelativeMouseMode(SDL_FALSE);

        inputDir = glm::dot(inputDir, inputDir) > 0.0f ? glm::normalize(inputDir) : inputDir; // dot(v, v) is squared length
        constexpr auto CAMERA_SPEED = 2.5f;
        cameraPos += inputDir * CAMERA_SPEED * fDeltaTime;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                default: break;
                case SDL_MOUSEWHEEL:
                    handle_scroll(event.wheel.y);
                    break;
                case SDL_MOUSEMOTION:
                    handle_mouse_movement(event, cameraFront);
                    break;
                case SDL_QUIT:
                    goto quit;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        progState.windowWidth = event.window.data1;
                        progState.windowHeight = event.window.data2;
                        glViewport(0, 0, progState.windowWidth, progState.windowHeight);
                    }
            }
        }

        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Render
        auto lightColor = glm::vec3(0.6f, 0.8f, 0.8f);

        shader.use();
        shader.setVec3("viewPos", cameraPos);

        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient",  0.1, 0.1, 0.1);
        shader.setVec3("light.diffuse",  0.6f, 0.8f, 0.8f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        shader.setInt("material.diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTex);
        shader.setInt("material.specular", 1);

        shader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(progState.fov),
            static_cast<float>(progState.windowWidth) / static_cast<float>(progState.windowHeight),
            0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", viewMatrix);

        // render the cubes
        glBindVertexArray(vao);
        // shader.setMat4("model", glm::mat4(1.0f));
        // glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / fStride);
        for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(glm::vec3); i++) {
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(20.0f * static_cast<float>(i)), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / fStride);
        }
        // // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        // render the light
        lightShader.use();
        lightShader.setVec3("color", lightColor);
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", viewMatrix);
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / fStride);

#pragma endregion

#ifdef DEBUG
        if (const GLenum error = glGetError(); error != GL_NO_ERROR) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: (%d) %s", error, glErrorString(error).c_str());
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

quit:
    GUI::shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
