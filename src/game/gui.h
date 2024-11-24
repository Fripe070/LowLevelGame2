#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include <engine/run.h>

struct GameState;

namespace DebugGUI {
    void init(SDL_Window &window, SDL_GLContext glContext);
    void shutdown();

    /*!
     * @brief Renders the debug GUI
     * @note Should be called AFTER all game rendering occurs, so that the debug GUI is drawn on top of everything
     */
    void render(GameState &gameState, StatePackage &statePackage);
    void handleEvent(const SDL_Event &event);
}


#endif //GUI_H
