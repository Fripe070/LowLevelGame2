#include "skybox.h"

#include <memory>
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "gui.h"
#include <game/state.h>

#include <engine/logging.h>
#include <glm/ext/matrix_transform.hpp>

#include "engine/game.h"

#include <glm/gtc/type_ptr.hpp>


std::unique_ptr<GameState> gameState;
unsigned int uboMatrices;

#define LEVEL gameState->level
#define PLAYER LEVEL.player
#define CAMERA PLAYER.camera

bool setupGame(StatePackage &statePackage, SDL_Window *sdlWindow, SDL_GLContext glContext) {
    gameState = std::make_unique<GameState>(statePackage);

    LEVEL.shaders.emplace_back("resources/assets/shaders/vert.vert", "resources/assets/shaders/frag.frag");
    LEVEL.shaders[0].use();
    auto matricesBinding = LEVEL.shaders[0].bindUniformBlock("Matrices", 0);
    if (!matricesBinding.has_value())
        logError("Failed to bind matrices uniform block" NL_INDENT "%s", matricesBinding.error().c_str());

    LEVEL.shaders.emplace_back("resources/assets/shaders/sb_vert.vert", "resources/assets/shaders/sb_frag.frag");
    LEVEL.shaders[1].use();
    matricesBinding = LEVEL.shaders[1].bindUniformBlock("Matrices", 0);
    if (!matricesBinding.has_value())
        logError("Failed to bind matrices uniform block" NL_INDENT "%s", matricesBinding.error().c_str());

    DebugGUI::init(*sdlWindow, glContext);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);  // Counter-clockwise winding order

    glEnable(GL_DEPTH_TEST);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Let these be managed by the camera class, so we only ever have to update the matrices when the camera moves/zooms
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
        glm::value_ptr(CAMERA.getProjectionMatrix(statePackage.windowSize->aspectRatio())));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
        glm::value_ptr(CAMERA.getViewMatrix()));

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

    LEVEL.shaders[0].setVec3("viewPos", CAMERA.position);

    auto scene = LEVEL.modelManager.getScene("resources/assets/models/map.obj");
    if (!scene.has_value())
        logError("Failed to load scene" NL_INDENT "%s", scene.error().c_str());
    auto drawRet = scene.value()->Draw(LEVEL.textureManager, LEVEL.shaders[0], glm::mat4(1.0f));
    if (!drawRet.has_value())
        logError("Failed to draw scene" NL_INDENT "%s", drawRet.error().c_str());

    const glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -2.0f));
    LEVEL.modelManager.errorScene->Draw(LEVEL.textureManager, LEVEL.shaders[0], trans);

#pragma region Skybox
    // We render the skybox manually, since we don't need any of the fancy scene stuff
    LEVEL.shaders[1].use();

    std::expected<unsigned int, std::string> skyboxTex = LEVEL.textureManager.getTexture(
        "resources/assets/textures/skybox/sky.png", Engine::Manager::TextureType::CUBEMAP);
    if (!skyboxTex.has_value())
        logError("Failed to load skybox texture" NL_INDENT "%s", skyboxTex.error().c_str());
    LEVEL.skybox.draw(skyboxTex.value_or(LEVEL.textureManager.errorTexture), LEVEL.shaders[1]);
#pragma endregion

    glDisable(GL_DEPTH_TEST);
    DebugGUI::render(*gameState, statePackage);
    glEnable(GL_DEPTH_TEST);  // Disabled in debug GUI rendering, so we re-enable it here

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
