#define DEBUG
#define STB_IMAGE_IMPLEMENTATION

#include <string>
#include <gl/glew.h>
#include <SDL.h>
#include <imgui_impl_sdl2.h>
#include "shader.h"
#include "gui.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <logging.h>
#include <scene_loader.h>
#include <utility.h>


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
        logError("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        progState.windowWidth, progState.windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        logError("Couldn't create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Modern OpenGL with core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        logError("Couldn't create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(window, glContext);

    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        logError("Couldn't initialize GLEW: %s", glewGetErrorString(glewError));
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_SetRelativeMouseMode(SDL_TRUE);

    GLuint diffuseTex = loadTexture("resources/brick.png").value();  // TODO: This will throw if the texture isn't found, but im removing it soon anyway
    GLuint specularTex = loadTexture("resources/brick_specular.png").value();

    const auto shader = Shader("resources/vert.vert", "resources/frag.frag");

    SceneLoader::Model ourModel("resources/test_mtl.obj");

    glEnable(GL_DEPTH_TEST);
    GUI::init(window, glContext);

    glm::mat4 viewMatrix;
    // TODO: Break camera into its own class
    auto cameraPos = glm::vec3(0.0f, 0.0f,  3.0f);
    auto cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    auto cameraUp = glm::vec3(0.0f, 1.0f,  0.0f);

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
        shader.use();

        shader.setVec3("viewPos", cameraPos);

        shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        shader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
        shader.setVec3("pointLights.diffuse", 0.4f, 0.4f, 0.4f);
        shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        shader.setVec3("pointLights[0].position", 1.2f, 1.0f, 2.0f);
        shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointLights[0].constant", 1.0f);
        shader.setFloat("pointLights[0].linear", 0.09f);
        shader.setFloat("pointLights[0].quadratic", 0.032f);

        shader.setVec3("spotLight.position", cameraPos);
        shader.setVec3("spotLight.direction", cameraFront);
        shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("spotLight.constant", 1.0f);
        shader.setFloat("spotLight.linear", 0.09f);
        shader.setFloat("spotLight.quadratic", 0.032f);
        shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

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

        shader.setMat4("model", glm::mat4(1.0f));
        ourModel.Draw(shader);

#pragma endregion

        glLogErrors();

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
