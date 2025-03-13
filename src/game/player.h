#ifndef PLAYER_H
#define PLAYER_H
#include <glm/vec3.hpp>

class Player {
public:
  glm::vec3 origin{};

  struct Rotation {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
  } rotation;
  [[nodiscard]] glm::vec3 getForward() const;
  [[nodiscard]] glm::vec3 getRight() const;
  [[nodiscard]] glm::vec3 getUp() const;

  bool tick();
};

#endif
