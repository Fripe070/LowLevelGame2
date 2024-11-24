#ifndef GAME_H
#define GAME_H
#include "engine/run.h"
#include <SDL_events.h>


bool setupGame(StatePackage &statePackage, SDL_Window *sdlWindow, SDL_GLContext glContext);
void shutdownGame(StatePackage &statePackage);

bool renderUpdate(double deltaTime, StatePackage &statePackage);
bool physicsUpdate(double deltaTime, StatePackage &statePackage);

bool handleEvent(const SDL_Event &event, StatePackage &statePackage);


#endif //GAME_H
