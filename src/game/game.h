#ifndef GAME_H
#define GAME_H
#include "engine/run.h"
#include <SDL_events.h>
#include <vector>
#include <engine/managers.h>
#include <engine/loader/model.h>
#include <engine/loader/shader.h>

#include "camera.h"


bool setupGame(StatePackage &statePackage, SDL_Window *sdlWindow, SDL_GLContext glContext);
void shutdownGame(StatePackage &statePackage);

bool renderUpdate(double deltaTime, StatePackage &statePackage);
bool physicsUpdate(double deltaTime, StatePackage &statePackage);

bool handleEvent(const SDL_Event &event, StatePackage &statePackage);

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
    std::vector<Engine::Loader::Model> models;
    std::vector<Engine::ShaderProgram> shaders;
    Engine::Manager::TextureManager textureManager;

    PlayerState player;

    explicit LevelState(const Settings settings): player(settings) {}
};

struct GameState {
    Settings settings;
    LevelState level;

    explicit GameState(StatePackage &statePackage): level(settings) {}
};


#endif //GAME_H
