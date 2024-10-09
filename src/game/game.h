#ifndef GAME_H
#define GAME_H
#include "engine/run.h"
#include <SDL_events.h>

bool setupGame();
void shutdownGame();

bool renderUpdate(double deltaTime, const WindowSize &windowSize);
bool physicsUpdate(double deltaTime);

bool handleEvent(const SDL_Event &event);

#endif //GAME_H
