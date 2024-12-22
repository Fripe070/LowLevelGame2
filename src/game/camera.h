#ifndef CAMERA_H
#define CAMERA_H
#include <SDL_events.h>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>

typedef float Radians;
typedef float Degrees;

auto constexpr DEFAULT_CAMERA_POSITION = glm::vec3(0.0f);
Degrees constexpr DEFAULT_FOV = 45.0f;
Radians constexpr DEFAULT_YAW = -90.0f;
Radians constexpr DEFAULT_PITCH = 0.0f;
Radians constexpr DEFAULT_ROLL = 0.0f;


class Camera {
public:
    glm::vec3 position;
    Degrees fov;

    float clipNear = 0.1f;
    float clipFar = 100.0f;

    explicit Camera(
        glm::vec3 position = DEFAULT_CAMERA_POSITION,
        Radians yaw = DEFAULT_YAW,
        Radians pitch = DEFAULT_PITCH,
        Radians roll = DEFAULT_ROLL,
        Degrees fov = DEFAULT_FOV
    );

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    // Witchcraft to not accept ints and only floating point types for aspectRatio
    template<typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<T>, glm::mat4>
    getProjectionMatrix(const T aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, clipNear, clipFar);
    }

private:
    bool anglesChanged = false;
    Radians yaw_;
    Radians pitch_;
    Radians roll_;

    glm::vec3 forward_;
    glm::vec3 up_;
    glm::vec3 right_;

    void updateVectors();

public:
    Radians yaw() const { return yaw_; }
    void setYaw(const Radians yaw) { yaw_ = yaw; anglesChanged = true; }
    Radians pitch() const { return pitch_; }
    void setPitch(const Radians pitch) { pitch_ = pitch; anglesChanged = true; }
    Radians roll() const { return roll_; }
    void setRoll(const Radians roll) { roll_ = roll; anglesChanged = true; }

#define UPDATE_IF_CHANGED() if (anglesChanged) { updateVectors(); anglesChanged = false; }
    glm::vec3 forward() { UPDATE_IF_CHANGED(); return forward_; }
    glm::vec3 up() { UPDATE_IF_CHANGED(); return up_; }
    glm::vec3 right() { UPDATE_IF_CHANGED(); return right_; }
#undef UPDATE_IF_CHANGED
};

class CameraController {
public:
    Camera &camera;

    float sensitivity = 0.1f;
    float maxPitch = 89.0f;
    float minPitch = -89.0f;

    explicit CameraController(Camera &camera, const float sensitivity)
        : camera(camera), sensitivity(sensitivity) {}
    explicit CameraController(Camera &camera, const float sensitivity, const float maxPitch, const float minPitch)
        : camera(camera), sensitivity(sensitivity), maxPitch(maxPitch), minPitch(minPitch) {}

    void look(const SDL_MouseMotionEvent &event) const;
    void zoom(const float offset) const;
};


#endif //CAMERA_H
