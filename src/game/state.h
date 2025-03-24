#pragma once

#include "engine/typedefs.h"
#include "game/player.h"

struct GameSettings {
    float sensitivity = 0.1f;
    Degrees baseFov = 45.0f;
    float clipNear = 0.1f;
    float clipFar = 100.0f;

    bool wireframe = false;
    bool backfaceCulling = true;
};

struct WorldState {
};

// TODO: Make state (or rather parts of it, smartly) savable (including engine state!!!)
struct GameState {
    GameSettings settings{};

    Player playerState{};
    WorldState worldState{};

    bool isPaused = false;
};

extern GameState *gameState;

