#include <memory>
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "gui.h"
#include <game/state.h>

#include "game.h"

#include <engine/logging.h>


std::unique_ptr<GameState> gameState;

#define LEVEL gameState->level
#define PLAYER LEVEL.player
#define CAMERA PLAYER.camera

bool setupGame(StatePackage &statePackage, SDL_Window *sdlWindow, SDL_GLContext glContext) {
    gameState = std::make_unique<GameState>(statePackage);

    LEVEL.shaders.emplace_back("resources/vert.vert", "resources/frag.frag");

    DebugGUI::init(*sdlWindow, glContext);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    return true;
}
void shutdownGame(StatePackage &statePackage) {
    gameState.reset();
}

bool renderUpdate(const double deltaTime, StatePackage &statePackage) {
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    auto inputDir = glm::vec3(0.0f, 0.0f,  0.0f);
    if (keyState[SDL_SCANCODE_W])
        inputDir += CAMERA.forward();
    if (keyState[SDL_SCANCODE_S])
        inputDir -= CAMERA.forward();
    if (keyState[SDL_SCANCODE_A])
        inputDir -= glm::normalize(glm::cross(CAMERA.forward(), CAMERA.up()));
    if (keyState[SDL_SCANCODE_D])
        inputDir += glm::normalize(glm::cross(CAMERA.forward(), CAMERA.up()));
    if (keyState[SDL_SCANCODE_SPACE])
        inputDir += CAMERA.up();
    if (keyState[SDL_SCANCODE_LSHIFT])
        inputDir -= CAMERA.up();
    if (keyState[SDL_SCANCODE_ESCAPE])
        SDL_SetRelativeMouseMode(SDL_FALSE);

    inputDir = glm::dot(inputDir, inputDir) > 0.0f ? glm::normalize(inputDir) : inputDir; // dot(v, v) is squared length
    constexpr auto CAMERA_SPEED = 2.5f;
    CAMERA.position += inputDir * CAMERA_SPEED * static_cast<float>(deltaTime);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    LEVEL.shaders[0].use();

    LEVEL.shaders[0].setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    LEVEL.shaders[0].setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
    LEVEL.shaders[0].setVec3("pointLights.diffuse", 0.4f, 0.4f, 0.4f);
    LEVEL.shaders[0].setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    LEVEL.shaders[0].setVec3("pointLights[0].position", 1.2f, 1.0f, 2.0f);
    LEVEL.shaders[0].setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    LEVEL.shaders[0].setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    LEVEL.shaders[0].setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    LEVEL.shaders[0].setFloat("pointLights[0].constant", 1.0f);
    LEVEL.shaders[0].setFloat("pointLights[0].linear", 0.09f);
    LEVEL.shaders[0].setFloat("pointLights[0].quadratic", 0.032f);

    LEVEL.shaders[0].setVec3("spotLight.position", CAMERA.position);
    LEVEL.shaders[0].setVec3("spotLight.direction", CAMERA.forward());
    LEVEL.shaders[0].setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    LEVEL.shaders[0].setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    LEVEL.shaders[0].setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    LEVEL.shaders[0].setFloat("spotLight.constant", 1.0f);
    LEVEL.shaders[0].setFloat("spotLight.linear", 0.09f);
    LEVEL.shaders[0].setFloat("spotLight.quadratic", 0.032f);
    LEVEL.shaders[0].setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    LEVEL.shaders[0].setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    LEVEL.shaders[0].setFloat("material.shininess", 32.0f);

    LEVEL.shaders[0].setVec3("viewPos", CAMERA.position);
    LEVEL.shaders[0].setMat4("projection", CAMERA.getProjectionMatrix(static_cast<float>(statePackage.windowSize->width) / statePackage.windowSize->height));
    LEVEL.shaders[0].setMat4("view", CAMERA.getViewMatrix());
    LEVEL.shaders[0].setMat4("model", glm::mat4(1.0f));

    auto scene = LEVEL.modelManager.getScene("resources/map.obj");
    if (!scene.has_value())
        logError("Failed to load scene: %s", scene.error().c_str());
    auto drawRet = scene.value().Draw(LEVEL.textureManager, LEVEL.shaders[0]);
    if (!drawRet.has_value())
        logError("Failed to draw scene: %s", drawRet.error().c_str());

    DebugGUI::render(*gameState, statePackage);

    return true;
}
bool physicsUpdate(const double deltaTime, StatePackage &statePackage) {
    return true;
}

bool handleEvent(const SDL_Event &event, StatePackage &statePackage) {
    DebugGUI::handleEvent(event);

    switch (event.type) {
        default: break;
        case SDL_MOUSEWHEEL:
            PLAYER.cController.zoom(event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
            PLAYER.cController.look(event.motion);
            break;
    }


    return false;
}
