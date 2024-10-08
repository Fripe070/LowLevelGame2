#ifndef GAME_H
#define GAME_H
#include <main.h>

bool setupGame();
void shutdownGame();

bool renderUpdate(double deltaTime, WindowSize &windowSize);
bool physicsUpdate(double deltaTime);

#endif //GAME_H
