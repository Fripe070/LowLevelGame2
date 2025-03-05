#include <GL/glew.h>
#include <SDL.h>
#include <cmath>

#include "run.h"
#include "logging.h"
#include "engine/game.h"

Config config;
WindowSize windowSize;

StatePackage statePackage = {&config, &windowSize};

int run()
{
#ifndef NDEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, LLG_GL_VER_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, LLG_GL_VER_MINOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifndef NDEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
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

#ifndef NDEBUG // OpenGL debug output
    int glCtxFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &glCtxFlags);
    if (glCtxFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        logDebug("OpenGL debug output enabled");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);
    } else {
        logWarn("OpenGL debug output not enabled");
    }
#endif

    if (!setupGame(statePackage, sdlWindow, glContext)) {
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
            const double expectedDT = 1.0 / config.maxFPS;
            if (config.limitFPS && !config.vsync && deltaTime < expectedDT) {
                SDL_Delay(static_cast<Uint32>((expectedDT - deltaTime) * 1000.0));
            }
#pragma endregion

            if (!(*statePackage.isPaused)) {
                physicsAccumulator += deltaTime;
                physicsAccumulator = std::fmin(physicsAccumulator, 0.1); // Prevent spiral of death  // TODO: Magic number?
                const double desiredPhysicsDT = 1.0 / config.physicsTPS;
                while (physicsAccumulator >= desiredPhysicsDT) {
                    if (!fixedUpdate(desiredPhysicsDT, statePackage)) {
                        logError("Physics update failed");
                        goto quit;
                    }
                    physicsAccumulator -= desiredPhysicsDT;
                }
            }

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (handleEvent(event, statePackage)) continue;

                switch (event.type) {
                    default: break;
                    case SDL_QUIT:
                        logDebug("Received quit signal");
                        goto quit;
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            windowSize.width = event.window.data1;
                            windowSize.height = event.window.data2;
                            glViewport(0, 0, windowSize.width, windowSize.height);
                        }
                        break;
                }
            }
            if (*statePackage.shouldRedraw || !*statePackage.isPaused) {
                const bool renderSuccess = renderUpdate(deltaTime, statePackage);
                glLogErrors();
                if (!renderSuccess) {
                    logError("Render update failed");
                    goto quit;
                }

                SDL_GL_SwapWindow(sdlWindow);
                *statePackage.shouldRedraw = false;
            }
            else {
                SDL_Delay(1);
            }
        }
    }

quit:
    shutdownGame(statePackage);
quitNoShutdown:
    logDebug("Shutting down");
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return 0;
}

