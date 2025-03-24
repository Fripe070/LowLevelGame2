#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // For reverse-z
#include <glm/mat4x4.hpp>

#include "game/state.h"

namespace CameraUtils {
    [[nodiscard]] glm::mat4 getViewMatrix(const Player &player);
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, float aspectRatio);
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, int width, int height);
    template<typename T>
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameState &gameState, T aspectRatio) = delete;
}

