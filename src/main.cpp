#include <gl/glew.h>
#include <SDL.h>

#include "main.h"
#include "engine/logging.h"

#include "game/game.h"

Config config;
WindowSize windowSize;

int main(int argv, char** args)
{
    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        logError("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *sdlWindow = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowSize.width, windowSize.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!sdlWindow) {
        logError("Couldn't create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Modern OpenGL with core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    const SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
    if (!glContext) {
        logError("Couldn't create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(sdlWindow, glContext);

    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        logError("Couldn't initialize GLEW: %s", glewGetErrorString(glewError));
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return -1;
    }

    if (!setupGame()) {
        logError("Setup failed");
        goto quitNoShutdown;
    }

    {
        double physicsAccumulator = 0.0;
        Uint64 frameStart = SDL_GetPerformanceCounter();
        while (true) {
#pragma region DeltaTime
            const Uint64 lastFrameStart = frameStart;
            frameStart = SDL_GetPerformanceCounter();
            double deltaTime = static_cast<double>(frameStart - lastFrameStart) / static_cast<double>(SDL_GetPerformanceFrequency());
            if (config.deltaTimeLimit > 0 && deltaTime > config.deltaTimeLimit) // Things may get a bit weird if our deltaTime is like 10 seconds
                deltaTime = config.deltaTimeLimit;

            // TODO: Process inputs instead of sleeping, to mitigate some input lag
            if (const double expectedDT = 1.0 / config.maxFPS;
                config.limitFPS && !config.vsync && deltaTime < expectedDT) {
                SDL_Delay(static_cast<Uint32>((expectedDT - deltaTime) * 1000.0));
                }
#pragma endregion

            physicsAccumulator += deltaTime;
            const double desiredPhysicsDT = 1.0 / config.physicsTPS;
            while (physicsAccumulator >= desiredPhysicsDT) {
                if (!physicsUpdate(desiredPhysicsDT)) {
                    logError("Physics update failed");
                    goto quit;
                }
                physicsAccumulator -= desiredPhysicsDT;
            }

            if (!renderUpdate(deltaTime, windowSize)) {
                logError("Render update failed");
                goto quit;
            }

            SDL_GL_SwapWindow(sdlWindow);
        }
    }

quit:
    shutdownGame();
quitNoShutdown:
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return 0;
}

