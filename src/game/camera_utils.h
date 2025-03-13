#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "game/state.h"

namespace CameraUtils {
    [[nodiscard]] glm::mat4 getViewMatrix(const Player &player);
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, float aspectRatio);
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, int width, int height);
    template<typename T>
    [[nodiscard]] glm::mat4 getProjectionMatrix(const GameState &gameState, T aspectRatio) = delete;
}

#endif
