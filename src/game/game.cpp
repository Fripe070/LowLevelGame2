#include "game.h"

#include <memory>
#include <gl/glew.h>
#include <engine/loader/model.h>
#include <glm/glm.hpp>

#include "camera.h"

std::unique_ptr<Engine::Loader::Model> model;
std::unique_ptr<Engine::Shader> shader;
std::unique_ptr<Camera> camera;
std::unique_ptr<CameraController> cController;

bool setupGame() {
    model = std::make_unique<Engine::Loader::Model>("resources/test_mtl.obj");
    shader = std::make_unique<Engine::Shader>("resources/vert.vert", "resources/frag.frag");
    camera = std::make_unique<Camera>();
    cController = std::make_unique<CameraController>(*camera, 0.1);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    return true;
}
void shutdownGame() {
    model.reset();
    shader.reset();
    camera.reset();
}

bool renderUpdate(const double deltaTime, const WindowSize &windowSize) {
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    auto inputDir = glm::vec3(0.0f, 0.0f,  0.0f);
    if (keyState[SDL_SCANCODE_W])
        inputDir += camera->forward();
    if (keyState[SDL_SCANCODE_S])
        inputDir -= camera->forward();
    if (keyState[SDL_SCANCODE_A])
        inputDir -= glm::normalize(glm::cross(camera->forward(), camera->up()));
    if (keyState[SDL_SCANCODE_D])
        inputDir += glm::normalize(glm::cross(camera->forward(), camera->up()));
    if (keyState[SDL_SCANCODE_SPACE])
        inputDir += camera->up();
    if (keyState[SDL_SCANCODE_LSHIFT])
        inputDir -= camera->up();
    if (keyState[SDL_SCANCODE_ESCAPE])
        SDL_SetRelativeMouseMode(SDL_FALSE);

    inputDir = glm::dot(inputDir, inputDir) > 0.0f ? glm::normalize(inputDir) : inputDir; // dot(v, v) is squared length
    constexpr auto CAMERA_SPEED = 2.5f;
    camera->position += inputDir * CAMERA_SPEED * static_cast<float>(deltaTime);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();

    shader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader->setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
    shader->setVec3("pointLights.diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    shader->setVec3("pointLights[0].position", 1.2f, 1.0f, 2.0f);
    shader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09f);
    shader->setFloat("pointLights[0].quadratic", 0.032f);

    shader->setVec3("spotLight.position", camera->position);
    shader->setVec3("spotLight.direction", camera->forward());
    shader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLight.constant", 1.0f);
    shader->setFloat("spotLight.linear", 0.09f);
    shader->setFloat("spotLight.quadratic", 0.032f);
    shader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setFloat("material.shininess", 32.0f);

    shader->setVec3("viewPos", camera->position);
    shader->setMat4("projection", camera->getProjectionMatrix(static_cast<float>(windowSize.width) / windowSize.height));
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("model", glm::mat4(1.0f));


    model->Draw(*shader);
    return true;
}
bool physicsUpdate(const double deltaTime) {
    return true;
}

bool handleEvent(const SDL_Event &event) {
    switch (event.type) {
        default: break;
        case SDL_MOUSEWHEEL:
            cController->zoom(event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
            cController->look(event.motion);
            break;
    }


    return false;
}
