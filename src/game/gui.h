#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include <engine/state.h>

struct EngineState;

namespace DebugGUI {
    void init();
    void shutdown();

    /*!
     * @brief Renders the debug GUI
     * @note Should be called AFTER all game rendering occurs, so that the debug GUI is drawn on top of everything
     */
    void render(double deltaTime);
    /*!
     * @brief Starts the rendering process for the debug GUI
     */
    void renderStart(double deltaTime);
    /*!
     * @brief Ends the rendering process for the debug GUI and renders it
     * @note Should be called AFTER all game rendering occurs
     */
    void renderEnd();

    void handleEvent(const SDL_Event &event);
}


#endif
