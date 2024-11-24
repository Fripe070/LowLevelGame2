#include "gui.h"


namespace DebugGUI {
    void init(SDL_Window &window, const SDL_GLContext glContext) {}
    void shutdown() {}
    void handleEvent(const SDL_Event &event) {}
    void drawFrame(GameState &gameState, StatePackage &statePackage);
    void render(GameState &gameState, StatePackage &statePackage) {}
    void drawFrame(GameState &gameState, StatePackage &statePackage) {}
}
