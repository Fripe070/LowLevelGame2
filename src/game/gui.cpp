#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <SDL.h>
#include <GL/glew.h>

#include "engine/run.h"
#include <game/state.h>

#include "gui.h"

#include <engine/state.h>


namespace DebugGUI {
    void init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplSDL2_InitForOpenGL(engineState->sdlWindow, engineState->glContext);
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

    void drawFrame(double deltaTime);

    void render(const double deltaTime) {
        renderStart(deltaTime);
        renderEnd();
    }
    void renderStart(const double deltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        drawFrame(deltaTime);
    }
    void renderEnd() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void drawSettingsGUI(double deltaTime);
    void drawOverlay(double deltaTime);

    void drawFrame(double deltaTime) {
        drawSettingsGUI(deltaTime);
        drawOverlay(deltaTime);

        // // TODO: fix menu not being visible on first pause
        // if (gameState.isPaused) {
        //     ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
        //                         ImGuiCond_Always, {0.5f, 0.5f});
        //     ImGui::Begin("Paused", nullptr,
        //             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        //     if (ImGui::Button("Resume")) { // does not work for now
        //         gameState.isPaused = false;
        //         *statePackage.shouldRedraw = true;
        //     }
        //     ImGui::End();
        // }

        // ImGui::ShowDemoWindow();
    }

    void drawSettingsGUI(double deltaTime) {
        ImGui::Begin("Settings");
        ImGui::Checkbox("Limit FPS", &engineState->config.limitFPS);
        if (engineState->config.limitFPS) {
            ImGui::Checkbox("VSync", &engineState->config.vsync);
            if (!engineState->config.vsync)
                ImGui::SliderInt("Max FPS", &engineState->config.maxFPS, 1, 300);
        }
        if (SDL_GL_SetSwapInterval(engineState->config.limitFPS && engineState->config.vsync ? -1 : 0) == -1)
            SDL_GL_SetSwapInterval(1);

        if (ImGui::CollapsingHeader("Mouse")) {
            ImGui::SliderFloat("Sensitivity", &gameState->settings.sensitivity, 0.01f, 1.0f);
        }
        if (ImGui::Button("Capture Mouse")) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            ImGui::SetWindowCollapsed(true);
        }
        ImGui::Checkbox("Wireframe", &gameState->settings.wireframe);
        ImGui::End();
    }

    void drawOverlay(double deltaTime) {
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
