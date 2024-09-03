#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <SDL.h>

#include "gui.h"

void GUI::init(SDL_Window* window, SDL_GLContext glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::frame(ProgState &progState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    showWindow(progState);
    // render();
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GUI::showWindow(ProgState &progState) {
    ImGui::Begin("Settings");
    ImGui::Checkbox("Limit FPS", &progState.limitFPS);
    if (progState.limitFPS) {
        ImGui::Checkbox("VSync", &progState.vsync);
        if (!progState.vsync)
            ImGui::SliderInt("Max FPS", &progState.maxFPS, 1, 300);
    }
    if (SDL_GL_SetSwapInterval(progState.limitFPS && progState.vsync ? -1 : 0) == -1)
        SDL_GL_SetSwapInterval(1);

    ImGui::End();
}
