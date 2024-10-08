#include "camera.h"

#include <glm/ext/matrix_transform.hpp>

Camera::Camera(const glm::vec3 position, const float yaw, const float pitch, const float roll, const float fov) {
    this->position = position;
    this->yaw_ = yaw;
    this->pitch_ = pitch;
    this->roll_ = roll;
    this->fov = fov;
    updateVectors();
}

void Camera::updateVectors() {
    forward_ = glm::normalize(glm::vec3(
        cos(glm::radians(yaw_)) * cos(glm::radians(pitch_)),
        sin(glm::radians(pitch_)),
        sin(glm::radians(yaw_)) * cos(glm::radians(pitch_))
    ));
    right_ = glm::normalize(glm::cross(forward_, glm::vec3(0.0f, 1.0f, 0.0f)));
    up_ = glm::normalize(glm::cross(right_, forward_));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + forward_, up_);
}

void CameraController::look(const SDL_MouseMotionEvent &event) const {
    const auto xOffset = static_cast<float>(event.xrel) * sensitivity;
    const auto yOffset = static_cast<float>(-event.yrel) * sensitivity;

    camera.setYaw(camera.yaw() + xOffset);
    camera.setPitch(std::max(minPitch, std::min(maxPitch, camera.pitch() + yOffset)));
}

void CameraController::zoom(const float offset) const {
    camera.fov -= offset;
    camera.fov = std::max(camera.fov, 0.1f);
    camera.fov = std::min(camera.fov, 160.0f);
    // for some reason, while using clamp it crashes on startup
    // progState.fov = std::clamp(progState.fov, 1.0f, 45.0f);
}
