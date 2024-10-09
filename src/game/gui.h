#ifndef GUI_H
#define GUI_H

#include <SDL.h>

typedef struct {
    bool limitFPS = true;
    double deltaTimeLimit = 3;
    bool vsync = true;
    int maxFPS = 100;
    float *sensitivity;
    bool wireframe = false;

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
