#ifndef RUN_H
#define RUN_H

#define LLG_GL_VER_MAJOR 4
#define LLG_GL_VER_MINOR 6

struct Config {
    double deltaTimeLimit = 3.0;
    bool limitFPS = true;
    bool vsync = true;
    int maxFPS = 100;
    int physicsTPS = 60;
};

struct WindowSize {
    int width = 1920 / 2;
    int height = 1080 / 2;

    [[nodiscard]] float aspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }
};

struct StatePackage {
    Config *config;
    WindowSize *windowSize;
    bool* isPaused;
    bool* shouldRedraw;
};

int run();

#endif //RUN_H
