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
    void render(GameState &gameState, StatePackage &statePackage, double deltaTime);
    /*!
     * @brief Starts the rendering process for the debug GUI
     */
    void renderStart(GameState &gameState, StatePackage &statePackage, double deltaTime);
    /*!
     * @brief Ends the rendering process for the debug GUI and renders it
     * @note Should be called AFTER all game rendering occurs
     */
    void renderEnd();

    void handleEvent(const SDL_Event &event);
}


#endif //GUI_H
