#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include <engine/run.h>

struct GameState;

namespace DebugGUI {
    void init(SDL_Window &window, SDL_GLContext glContext);
    void shutdown();

    void render(GameState &gameState, StatePackage &statePackage);
    void handleEvent(const SDL_Event &event);
}


#endif //GUI_H
