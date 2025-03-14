#include "camera_utils.h"

#include <glm/gtc/matrix_transform.hpp>


namespace CameraUtils {
    glm::mat4 getViewMatrix(const Player &player) {
        return glm::lookAt(
            player.origin,
            player.origin + player.getForward(),
            player.getUp());
    }

    glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, const float aspectRatio) {
        return glm::perspective(glm::radians(gameSettings.baseFov), aspectRatio,
            // Swapped NEAR and FAR for reverse-z
            gameSettings.clipFar, gameSettings.clipNear);
    }
    glm::mat4 getProjectionMatrix(const GameSettings &gameSettings, const int width, const int height) {
        return getProjectionMatrix(gameSettings, static_cast<float>(width) / static_cast<float>(height));
    }

}
