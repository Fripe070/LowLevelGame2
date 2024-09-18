#ifndef GUI_H
#define GUI_H

#include <SDL.h>

typedef struct {
    bool limitFPS = true;
    bool vsync = true;
    int maxFPS = 100;
    float rotX = 0.0f;
    float rotY = 0.0f;
    float rotZ = 0.0f;
    float fov = 45.0f;
    float sensitivity = 0.1f;

    int windowWidth = 1920 / 2;
    int windowHeight = 1080 / 2;
} ProgState;

class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    static void init(SDL_Window *window, SDL_GLContext glContext);
    static void frame(ProgState &progState);
    static void render();
    static void shutdown();
    static void showWindow(ProgState &progState);
};

#endif //GUI_H
