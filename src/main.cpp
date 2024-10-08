#define DEBUG
#define STB_IMAGE_IMPLEMENTATION

#include <camera.h>
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
Camera camera;
CameraController cController(camera, 0.1);

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

    progState.sensitivity = &cController.sensitivity;

    Uint64 frameStart = SDL_GetPerformanceCounter();
    SDL_Event event;
    while (true) {
        const Uint64 lastFrameStart = frameStart;
        frameStart = SDL_GetPerformanceCounter();
        double deltaTime = static_cast<double>(frameStart - lastFrameStart) / static_cast<double>(SDL_GetPerformanceFrequency());
        if (progState.deltaTimeLimit > 0 && deltaTime > progState.deltaTimeLimit) // Things may get a bit weird if our deltaTime is like 10 seconds
            deltaTime = progState.deltaTimeLimit;

        const auto fDeltaTime = static_cast<float>(deltaTime); // We need it as a float a lot

        // TODO: Process inputs instead of sleeping, to mitigate some input lag
        if (const double expectedDT = 1.0 / progState.maxFPS;
            progState.limitFPS && !progState.vsync && deltaTime < expectedDT) {
            SDL_Delay(static_cast<Uint32>((expectedDT - deltaTime) * 1000.0));
        }

        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        auto inputDir = glm::vec3(0.0f, 0.0f,  0.0f);
        if (keyState[SDL_SCANCODE_W])
            inputDir += camera.forward();
        if (keyState[SDL_SCANCODE_S])
            inputDir -= camera.forward();
        if (keyState[SDL_SCANCODE_A])
            inputDir -= glm::normalize(glm::cross(camera.forward(), camera.up()));
        if (keyState[SDL_SCANCODE_D])
            inputDir += glm::normalize(glm::cross(camera.forward(), camera.up()));
        if (keyState[SDL_SCANCODE_SPACE])
            inputDir += camera.up();
        if (keyState[SDL_SCANCODE_LSHIFT])
            inputDir -= camera.up();
        if (keyState[SDL_SCANCODE_ESCAPE])
            SDL_SetRelativeMouseMode(SDL_FALSE);

        inputDir = glm::dot(inputDir, inputDir) > 0.0f ? glm::normalize(inputDir) : inputDir; // dot(v, v) is squared length
        constexpr auto CAMERA_SPEED = 2.5f;
        camera.position += inputDir * CAMERA_SPEED * fDeltaTime;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                default: break;
                case SDL_MOUSEWHEEL:
                    cController.zoom(event.wheel.y);
                    break;
                case SDL_MOUSEMOTION:
                    cController.look(event.motion);
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Render
        shader.use();

        shader.setVec3("viewPos", camera.position);

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

        shader.setVec3("spotLight.position", camera.position);
        shader.setVec3("spotLight.direction", camera.forward());
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

        shader.setMat4("projection", camera.getProjectionMatrix(progState.windowWidth / static_cast<float>(progState.windowHeight)));
        shader.setMat4("view", camera.getViewMatrix());
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
