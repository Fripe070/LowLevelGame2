#ifndef PLAYER_H
#define PLAYER_H
#include <engine/typedefs.h>
#include <glm/vec3.hpp>

class Player {
public:
  glm::vec3 origin{};

  struct Rotation {
    Degrees yaw = 0.0f;
    Degrees pitch = 0.0f;
    Degrees roll = 0.0f;
  } rotation;
  [[nodiscard]] glm::vec3 getForward() const;
  [[nodiscard]] glm::vec3 getRight() const;
  [[nodiscard]] glm::vec3 getUp() const;

  bool tick();
};

#endif
