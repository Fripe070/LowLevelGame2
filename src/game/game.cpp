#include "engine/game.h"

#include <imgui.h>
#include <memory>
#include <engine/resources/scene.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera_utils.h"
#include "gui.h"
#include "skybox.h"
#include "state.h"
#include "engine/resources/resource_manager.h"
#include "engine/state.h"
#include "engine/render/frame_buffer.h"
#include "engine/util/logging.h"

GameState *gameState;

unsigned int uboMatrices;

std::unique_ptr<FrameBuffer> frameBuffer;

// This is TEMPORARY until I // TODO: Implement a concept of objects/levels/whatever
std::vector<std::shared_ptr<Resource::Scene>> scenes;
Skybox *skybox;
std::shared_ptr<Resource::Shader> mainShader;

bool setupGame() {
    gameState = new GameState();

    DebugGUI::init();
    int windowWidth, windowHeight;
    SDL_GL_GetDrawableSize(engineState->sdlWindow, &windowWidth, &windowHeight);
    frameBuffer = std::make_unique<FrameBuffer>(windowWidth, windowHeight);

    if (gameState->settings.backfaceCulling)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glFrontFace(GL_CCW);  // Counter-clockwise winding order
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);  // OpenGL's default NDC is [-1, 1], we want [0, 1]

    SDL_SetRelativeMouseMode(SDL_TRUE);
    mainShader = engineState->resourceManager.loadShader(
        "resources/assets/shaders/vert.vert", "resources/assets/shaders/frag.frag");;
    mainShader->use();
    auto matricesBinding = mainShader->bindUniformBlock("Matrices", 0);
    if (!matricesBinding.has_value())
        throw std::runtime_error(stringifyError(FW_ERROR(matricesBinding.error(), "Failed to bind matrices uniform block")));

    // TODO: So super duper mega ultra scuffed and a remnant from when we initialised the skybox stuff manually each frame
    const std::shared_ptr<Resource::Shader> sbShader = engineState->resourceManager.loadShader(
        "resources/assets/shaders/sb_vert.vert", "resources/assets/shaders/sb_frag.frag");
    sbShader->use();
    matricesBinding = sbShader->bindUniformBlock("Matrices", 0);
    if (!matricesBinding.has_value())
        throw std::runtime_error(stringifyError(FW_ERROR(matricesBinding.error(), "Failed to bind matrices uniform block")));
    skybox = new Skybox(engineState->resourceManager.loadCubemap("resources/assets/textures/skybox/sky.png"));

    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    scenes.push_back(engineState->resourceManager.loadScene("resources/assets/models/map.obj"));

    return true;
}
void shutdownGame() {
    DebugGUI::shutdown();
    glDeleteBuffers(1, &uboMatrices);
    delete gameState;
    delete skybox;
}
bool pausedRenderUpdate(double deltaTime);

bool renderUpdate(const double deltaTime) {
    if (gameState->isPaused)
        return pausedRenderUpdate(deltaTime);

    int windowWidth, windowHeight;
    SDL_GL_GetDrawableSize(engineState->sdlWindow, &windowWidth, &windowHeight);

    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    auto inputDir = glm::vec3(0.0f, 0.0f,  0.0f);
    if (keyState[SDL_SCANCODE_W])
        inputDir += gameState->playerState.getForward();
    if (keyState[SDL_SCANCODE_S])
        inputDir -= gameState->playerState.getForward();
    if (keyState[SDL_SCANCODE_A])
        inputDir -= gameState->playerState.getRight();
    if (keyState[SDL_SCANCODE_D])
        inputDir += gameState->playerState.getRight();
    if (keyState[SDL_SCANCODE_SPACE])
        inputDir += gameState->playerState.getUp();
    if (keyState[SDL_SCANCODE_LSHIFT])
        inputDir -= gameState->playerState.getUp();
    if (keyState[SDL_SCANCODE_F])
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(
            !SDL_GetRelativeMouseMode()));

    inputDir = glm::dot(inputDir, inputDir) > 0.0f ? glm::normalize(inputDir) : inputDir; // dot(v, v) is squared length
    constexpr auto CAMERA_SPEED = 2.5f;
    gameState->playerState.origin += inputDir * CAMERA_SPEED * static_cast<float>(deltaTime);

    frameBuffer->bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glClearColor(0.62, 0.56, 0.95, 1);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, gameState->settings.wireframe ? GL_LINE : GL_FILL);
    // TODO: Allow backface culling to be toggled per object
    // Don't cull if in wireframe mode
    if (gameState->settings.wireframe)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
        glm::value_ptr(CameraUtils::getProjectionMatrix(gameState->settings, windowWidth, windowHeight)));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
        glm::value_ptr(CameraUtils::getViewMatrix(gameState->playerState)));

    mainShader->use();

    mainShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    mainShader->setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
    mainShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    mainShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    mainShader->setVec3("pointLights[0].position", 1.2f, 1.0f, 2.0f);
    mainShader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    mainShader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    mainShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    mainShader->setFloat("pointLights[0].constant", 1.0f);
    mainShader->setFloat("pointLights[0].linear", 0.09f);
    mainShader->setFloat("pointLights[0].quadratic", 0.032f);

    mainShader->setVec3("spotLight.position", gameState->playerState.origin);
    mainShader->setVec3("spotLight.direction", gameState->playerState.getForward());
    mainShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    mainShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    mainShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    mainShader->setFloat("spotLight.constant", 1.0f);
    mainShader->setFloat("spotLight.linear", 0.09f);
    mainShader->setFloat("spotLight.quadratic", 0.032f);
    mainShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    mainShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    // TODO: Why is this not handled in the buffer
    mainShader->setVec3("viewPos", gameState->playerState.origin);

    for (const auto &scene : scenes) {
        auto drawRet = scene->Draw();
        if (!drawRet.has_value())
            reportError(FW_ERROR(drawRet.error(), "Failed to draw scene"));
    }

    const glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -2.0f));
    engineState->resourceManager.loadScene("INVALID_SCENE")->Draw(trans);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#pragma region Skybox
    skybox->draw();
#pragma endregion

#pragma region "Transfer color buffer to the default framebuffer before rendering overlays"
    frameBuffer->bind(GL_READ_FRAMEBUFFER);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, frameBuffer->getSize().width, frameBuffer->getSize().height,
        0, 0, windowWidth, windowHeight,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);
    // Make sure both read and write are set to the default framebuffer. Theoretically only binding read would do.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma region "Render overlays"
    glDepthFunc(GL_LESS);  // Don't leak reverse z into other rendering, IDK what imgui or whatever might be doing
    glDisable(GL_DEPTH_TEST);

    DebugGUI::renderStart(deltaTime);

    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Color buffer");
    ImGui::Image(frameBuffer->ColorTextureID,
        ImVec2(static_cast<float>(windowWidth) / 4, static_cast<float>(windowHeight) / 4),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    DebugGUI::renderEnd();
#pragma endregion

    SDL_GL_SwapWindow(engineState->sdlWindow);
    return true;
}

bool pausedRenderUpdate(const double deltaTime) {
    return true;
}

bool fixedUpdate(const double deltaTime) {
    if (gameState->isPaused) return true;

    return true;
}

bool handleEvent(const SDL_Event &event) {
    DebugGUI::handleEvent(event);

    switch (event.type) {
        default: break;
        case SDL_KEYDOWN:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                gameState->isPaused ^= true;
                SDL_SetRelativeMouseMode(static_cast<SDL_bool>(!gameState->isPaused));
                return true;
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_P) {
                SDL_SetRelativeMouseMode(static_cast<SDL_bool>(!SDL_GetRelativeMouseMode()));
                return true;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (!gameState->isPaused) {
                gameState->settings.baseFov -= static_cast<float>(event.wheel.y) * 2.0f;
                // TODO: Don't adjust this directly when zooming, instead change a modifier on top of it
                gameState->settings.baseFov = std::max(gameState->settings.baseFov, 0.1f);
                gameState->settings.baseFov = std::min(gameState->settings.baseFov, 160.0f);
            }
            break;
        case SDL_MOUSEMOTION:
            if (!gameState->isPaused && SDL_GetRelativeMouseMode()) {
                const auto xOffset = static_cast<float>(event.motion.xrel) * gameState->settings.sensitivity;
                const auto yOffset = static_cast<float>(-event.motion.yrel) * gameState->settings.sensitivity;
                gameState->playerState.rotation.yaw += xOffset;
                gameState->playerState.rotation.pitch = std::max(-89.0f, std::min(89.0f,
                    gameState->playerState.rotation.pitch + yOffset));
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                // TODO: Move framebuffer handling to the engine
                int w, h;
                SDL_GL_GetDrawableSize(engineState->sdlWindow, &w, &h);
                frameBuffer->resize(event.window.data1, event.window.data2);
            }
            break;
    }

    return false;
}
