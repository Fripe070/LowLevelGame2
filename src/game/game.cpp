#include "game.h"

#include <memory>
#include <gl/glew.h>
#include <engine/loader/model.h>
#include <engine/loader/shader.h>
#include "engine/managers.h"
#include <glm/glm.hpp>

#include "camera.h"


struct PlayerState {
    Camera camera;
    CameraController cController = CameraController(camera, 0.1);
};


struct LevelState {
    std::vector<Engine::Loader::Model> models;
    std::vector<Engine::ShaderProgram> shaders;
    Engine::Manager::TextureManager textureManager;

    PlayerState player;
};

struct GameState {
    LevelState level;
};

std::unique_ptr<GameState> gameState;

#define LEVEL gameState->level
#define PLAYER LEVEL.player
#define CAMERA PLAYER.camera

bool setupGame() {
    gameState = std::make_unique<GameState>();

    LEVEL.models.emplace_back("resources/map.obj");
    LEVEL.shaders.emplace_back("resources/vert.vert", "resources/frag.frag");

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    return true;
}
void shutdownGame() {
    gameState.reset();
}


bool renderUpdate(const double deltaTime, const WindowSize &windowSize) {
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
    LEVEL.shaders[0].setMat4("projection", CAMERA.getProjectionMatrix(static_cast<float>(windowSize.width) / windowSize.height));
    LEVEL.shaders[0].setMat4("view", CAMERA.getViewMatrix());
    LEVEL.shaders[0].setMat4("model", glm::mat4(1.0f));

    for (auto &model : LEVEL.models)
        model.Draw(LEVEL.textureManager, LEVEL.shaders[0]);
    return true;
}
bool physicsUpdate(const double deltaTime) {
    return true;
}

bool handleEvent(const SDL_Event &event) {
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
