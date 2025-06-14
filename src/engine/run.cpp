#include "run.h"

#include <GL/glew.h>
#include <SDL.h>
#include <cmath>

#include "engine/util/logging.h"
#include "engine/game.h"
#include "engine/state.h"

EngineState *engineState;

int run()
{
#pragma region Setup
    setupLogging();
    SDL_LogSetOutputFunction(LogSDLCallback, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SPDLOG_ERROR("Couldn't initialize SDL: {}", SDL_GetError());
        return 1;
    }

    SDL_Window *sdlWindow = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1920/2, 1080/2,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!sdlWindow) {
        SPDLOG_ERROR("Couldn't create window: {}", SDL_GetError());
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
        SPDLOG_ERROR("Couldn't create OpenGL context: {}", SDL_GetError());
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(sdlWindow, glContext);

    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        // SPDLOG_ERROR("Couldn't initialize GLEW: {}", glewGetErrorString(glewError));
        SPDLOG_ERROR(fmt::format("Couldn't initialize GLEW: {}", std::string(reinterpret_cast<const char*>(glewGetErrorString(glewError)))));
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return -1;
    }

#ifndef NDEBUG // OpenGL debug output
    int glCtxFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &glCtxFlags);
    if (glCtxFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        SPDLOG_DEBUG("OpenGL debug output enabled");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(LogGLCallback, nullptr);
    } else {
        SPDLOG_WARN("OpenGL debug output not available");
    }
#endif

    // Loaded enough to create the global state
    engineState = new EngineState(sdlWindow, glContext);
    Expected<void> managerResult = engineState->resourceManager.populateErrorResources();
    if (!managerResult.has_value())
        throw std::runtime_error(stringifyError(FW_ERROR(managerResult.error(),
            "Failed to load resource manager error resources")));

    if (!setupGame()) {
        SPDLOG_ERROR("Setup failed");
        goto quitNoShutdown;
    }
#pragma endregion

#pragma region MainLoop
    {
        double fixedAccumulator = 0.0;
        Uint64 frameStart = SDL_GetPerformanceCounter();
        while (true) {
#pragma region DeltaTime
            const Uint64 lastFrameStart = frameStart;
            frameStart = SDL_GetPerformanceCounter();
            double deltaTime = static_cast<double>(frameStart - lastFrameStart) / static_cast<double>(SDL_GetPerformanceFrequency());
            if (engineState->config.deltaTimeLimit > 0 && deltaTime > engineState->config.deltaTimeLimit) // Things may get a bit weird if our deltaTime is like 10 seconds
                deltaTime = engineState->config.deltaTimeLimit;

            // TODO: Process inputs instead of sleeping, to mitigate some input lag
            const double expectedDT = 1.0 / engineState->config.maxFPS;
            if (engineState->config.limitFPS && !engineState->config.vsync && deltaTime < expectedDT) {
                SDL_Delay(static_cast<Uint32>((expectedDT - deltaTime) * 1000.0));
            }
#pragma endregion

            fixedAccumulator += deltaTime;
            fixedAccumulator = std::fmin(fixedAccumulator, 0.1); // Prevent spiral of death  // TODO: Magic number?
            const double desiredFixedDT = 1.0 / engineState->config.fixedTPS;

            while (fixedAccumulator >= desiredFixedDT) {
                if (!fixedUpdate(desiredFixedDT)) {
                    SPDLOG_ERROR("Fixed update failed");
                    goto quit;
                }
                fixedAccumulator -= desiredFixedDT;
            }

            // TODO: Allow recording demos?
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (handleEvent(event)) continue;

                switch (event.type) {
                    default: break;
                    case SDL_QUIT:
                        SPDLOG_DEBUG("Received quit signal");
                        goto quit;
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            int w, h;
                            SDL_GL_GetDrawableSize(sdlWindow, &w, &h);
                            glViewport(0, 0, w, h);
                        }
                        break;
                }
            }

            const bool renderSuccess = renderUpdate(deltaTime);
            glLogErrors();
            if (!renderSuccess) {
                SPDLOG_ERROR("Render update failed");
                goto quit;
            }
        }
    }
#pragma endregion

#pragma region Shutdown
quit:
    shutdownGame();
quitNoShutdown:
    delete engineState;
    SPDLOG_DEBUG("Shutting down");
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
#pragma endregion

    return 0;
}

