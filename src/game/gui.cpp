#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <SDL.h>
#include <GL/glew.h>

#include "gui.h"

#include "game.h"


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

    void render(GameState &gameState, StatePackage &statePackage) {
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



// void GUI::frame(const GameState &gameState) {
//     ImGui_ImplOpenGL3_NewFrame();
//     ImGui_ImplSDL2_NewFrame();
//     ImGui::NewFrame();
//     showWindow(gameState);
//     // render();
// }
//
// void GUI::render() {
//     ImGui::Render();
//     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
// }
//
// void GUI::shutdown() {
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplSDL2_Shutdown();
//     ImGui::DestroyContext();
// }
//
// void GUI::showWindow(const GameState &gameState) {
//     gameState.settings.showWindow();
//
//     ImGui::Begin("Settings");
//     ImGui::Checkbox("Limit FPS", &progState.limitFPS);
//     if (progState.limitFPS) {
//         ImGui::Checkbox("VSync", &progState.vsync);
//         if (!progState.vsync)
//             ImGui::SliderInt("Max FPS", &progState.maxFPS, 1, 300);
//     }
//     if (SDL_GL_SetSwapInterval(progState.limitFPS && progState.vsync ? -1 : 0) == -1)
//         SDL_GL_SetSwapInterval(1);
//
//     if (ImGui::CollapsingHeader("Mouse")) {
//         ImGui::SliderFloat("Sensitivity", progState.sensitivity, 0.01f, 1.0f);
//     }
//     if (ImGui::Button("Capture Mouse")) {
//         SDL_SetRelativeMouseMode(SDL_TRUE);
//         ImGui::SetWindowCollapsed(true);
//     }
//     if (ImGui::Checkbox("Wireframe", &progState.wireframe)) {
//         glPolygonMode(GL_FRONT_AND_BACK, progState.wireframe ? GL_LINE : GL_FILL);
//     }
//
//     ImGui::End();
// }
