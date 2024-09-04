#pragma once

#include <SDL.h>

typedef struct {
    bool limitFPS = true;
    bool vsync = true;
    int maxFPS = 100;
    float rotX = 0.0f;
    float rotY = 0.0f;
    float rotZ = 0.0f;
} ProgState;

class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    static void init(SDL_Window* window, SDL_GLContext glContext);
    static void frame(ProgState &progState);
    static void render();
    static void shutdown();
    static void showWindow(ProgState &progState);
};
