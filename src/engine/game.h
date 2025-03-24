#pragma once
#include <SDL_events.h>

bool setupGame();
void shutdownGame();

bool renderUpdate(double deltaTime);
bool fixedUpdate(double deltaTime);

bool handleEvent(const SDL_Event &event);


