#pragma once
#include <SDL_video.h>

#include "manager/scene.h"
#include "manager/texture.h"

struct EngineConfig {
    double deltaTimeLimit = 3.0;
    bool limitFPS = true;
    bool vsync = true;
    int maxFPS = 100;
    int physicsTPS = 60;
};

struct EngineState {
    EngineState(SDL_Window *sdlWindow, void *glContext)
        : sdlWindow(sdlWindow), glContext(glContext) {}

    SDL_Window *sdlWindow;
    void *glContext;

    EngineConfig config{};

    Engine::Manager::SceneManager sceneManager{};
    Engine::Manager::TextureManager textureManager{};
};

/*!
 * @brief The global state of the engine
 */
extern EngineState *engineState; // TODO: Ref not ptr?

