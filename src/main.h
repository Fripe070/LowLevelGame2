#ifndef RUNNER_H
#define RUNNER_H

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
};

int run(int argv, char** args);

#endif //RUNNER_H
