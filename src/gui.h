#pragma once

#include <SDL.h>

class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    static void init(SDL_Window* window, SDL_GLContext glContext);
    void frame();
    static void render();
    static void shutdown();
    void showWindow();

private:
    bool checkboxState = false;
};
