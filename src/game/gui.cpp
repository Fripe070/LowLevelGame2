#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <SDL.h>
#include <GL/glew.h>

#include "game.h"
#include <game/state.h>

#include "gui.h"


namespace DebugGUI {
    void init(SDL_Window &window, const SDL_GLContext glContext) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplSDL2_InitForOpenGL(&window, glContext);
        ImGui_ImplOpenGL3_Init();
    }
    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void handleEvent(const SDL_Event &event) {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    void drawFrame(GameState &gameState, StatePackage &statePackage);

    /*!
     * @brief Renders the debug GUI
     * @note Should be called AFTER all game rendering occurs, so that the debug GUI is drawn on top of everything
     * @note Calling disables depth testing
     */
    void render(GameState &gameState, StatePackage &statePackage) {
        glDisable(GL_DEPTH_TEST);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();
        drawFrame(gameState, statePackage);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void drawFrame(GameState &gameState, StatePackage &statePackage) {
#define ENGINE_CONFIG statePackage.config
#define GAME_SETTINGS gameState.settings

        ImGui::Begin("Settings");
        ImGui::Checkbox("Limit FPS", &ENGINE_CONFIG->limitFPS);
        if (ENGINE_CONFIG->limitFPS) {
            ImGui::Checkbox("VSync", &ENGINE_CONFIG->vsync);
            if (!ENGINE_CONFIG->vsync)
                ImGui::SliderInt("Max FPS", &ENGINE_CONFIG->maxFPS, 1, 300);
        }
        if (SDL_GL_SetSwapInterval(ENGINE_CONFIG->limitFPS && ENGINE_CONFIG->vsync ? -1 : 0) == -1)
            SDL_GL_SetSwapInterval(1);

        if (ImGui::CollapsingHeader("Mouse")) {
            ImGui::SliderFloat("Sensitivity", &GAME_SETTINGS.sensitivity, 0.01f, 1.0f);
        }
        if (ImGui::Button("Capture Mouse")) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            ImGui::SetWindowCollapsed(true);
        }
        if (ImGui::Checkbox("Wireframe", &GAME_SETTINGS.wireframe)) {
            glPolygonMode(GL_FRONT_AND_BACK, GAME_SETTINGS.wireframe ? GL_LINE : GL_FILL);
        }

        ImGui::End();
    }
}
