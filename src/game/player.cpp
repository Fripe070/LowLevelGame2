#include "player.h"

#include <glm/detail/func_geometric.inl>
#include <glm/detail/func_trigonometric.inl>


glm::vec3 Player::getForward() const {
    const auto xz = cos(glm::radians(this->rotation.pitch));
    return glm::normalize(glm::vec3(
        cos(glm::radians(this->rotation.yaw)) * cos(glm::radians(this->rotation.pitch)),
        sin(glm::radians(this->rotation.pitch)),
        sin(glm::radians(this->rotation.yaw)) * cos(glm::radians(this->rotation.pitch))
    ));
}
glm::vec3 Player::getRight() const {
    return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}
glm::vec3 Player::getUp() const {
    return glm::normalize(glm::cross(getRight(), getForward()));
}




