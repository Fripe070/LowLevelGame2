#ifndef STATE_H
#define STATE_H

#include <vector>
#include <engine/manager/texture.h>
#include <engine/manager/scene.h>
#include <engine/loader/shader.h>

#include "camera.h"

struct Settings {
    // Mouse
    float sensitivity = 0.1f;
    // Graphics
    bool wireframe = false;
};

struct PlayerState {
    Camera camera;
    CameraController cController;

    explicit PlayerState(const Settings settings): cController(camera, settings.sensitivity) {}
};

struct LevelState {
    std::vector<Engine::ShaderProgram> shaders;
    Engine::Manager::TextureManager textureManager;
    Engine::Manager::SceneManager modelManager;

    std::vector<std::string> modelPaths;

    PlayerState player;

    explicit LevelState(const Settings settings): player(settings) {}
};

struct GameState {
    Settings settings;
    LevelState level;

    explicit GameState(StatePackage &statePackage): settings(), level(settings) {}
};


#endif //STATE_H
