#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <SDL.h>
#include <GL/glew.h>

#include "engine/run.h"
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

    void drawFrame(GameState &gameState, StatePackage &statePackage, double deltaTime);

    void render(GameState &gameState, StatePackage &statePackage, const double deltaTime) {
        renderStart(gameState, statePackage, deltaTime);
        renderEnd();
    }
    void renderStart(GameState &gameState, StatePackage &statePackage, double deltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        drawFrame(gameState, statePackage, deltaTime);
    }
    void renderEnd() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


    void drawSettingsGUI(GameState &gameState, StatePackage &statePackage, double deltaTime);
    void drawOverlay(GameState &gameState, StatePackage &statePackage, double deltaTime);

    void drawFrame(GameState &gameState, StatePackage &statePackage, double deltaTime) {
        drawSettingsGUI(gameState, statePackage, deltaTime);
        drawOverlay(gameState, statePackage, deltaTime);

        // ImGui::ShowDemoWindow();
    }

    void drawSettingsGUI(GameState &gameState, StatePackage &statePackage, double deltaTime) {
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

    void drawOverlay(GameState &gameState, StatePackage &statePackage, double deltaTime) {
        constexpr auto flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove;
        {
            // Lock on to top right
            constexpr float PADDING = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            const ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            const ImVec2 work_size = viewport->WorkSize;
            ImGui::SetNextWindowPos(
                {work_pos.x + work_size.x - PADDING,
                    work_pos.y + PADDING},
                ImGuiCond_Always,
                {1.0f, 0.0f}
            );
        }
        ImGui::SetNextWindowBgAlpha(0.9);
        ImGui::Begin("Overlay", nullptr, flags);

        const auto fps = 1.0 / deltaTime;
        auto col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (fps < 30)
            col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        else if (fps < 60)
            col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        else if (fps < 120)
            col = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImGui::TextColored(col, "%.0f FPS (%.1f ms)", fps, deltaTime * 1000.0);
        ImGui::End();
    }
}
