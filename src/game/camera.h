#ifndef CAMERA_H
#define CAMERA_H
#include <SDL_events.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // For reverse-z
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

    // TODO: Only ever have to update the matrices when the camera moves/zooms.
    //  Or maybe just simplify this entire monster of a class and eat the microscopical (potential) performance hit...
    //  I haven't actually even profiled so that seems most reasonable xD
    /*!
     * @brief Update the UBO holding the projection and view matrices.
     * @param uboID The ID of the uniform buffer object.
     * @param aspectRatio The aspect ratio of the window. Must be a floating point type (float, double).
    */
    void populateProjMatrixBuffer(unsigned int uboID, float aspectRatio) const;

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    // Witchcraft to not accept ints and only floating point types for aspectRatio
    template<typename FloatOnly>
    [[nodiscard]]
    std::enable_if_t<std::is_floating_point_v<FloatOnly>, glm::mat4>
    /*!
     * @brief Get the projection matrix for the camera.
     * @param aspectRatio The aspect ratio of the window. Must be a floating point type (float, double).
     * @return The projection matrix.
     */
    getProjectionMatrix(const FloatOnly aspectRatio) const {
        // Swapped near and far for reverse-z
        return glm::perspective(glm::radians(fov), aspectRatio, clipFar, clipNear);
    }

private:
    bool anglesChanged = false;
    Radians yaw_;
    Radians pitch_;
    Radians roll_;

    glm::vec3 forward_{};
    glm::vec3 up_{};
    glm::vec3 right_{};

    void updateVectors();

public:
    [[nodiscard]] Radians yaw() const { return yaw_; }
    void setYaw(const Radians yaw) { yaw_ = yaw; anglesChanged = true; }
    [[nodiscard]] Radians pitch() const { return pitch_; }
    void setPitch(const Radians pitch) { pitch_ = pitch; anglesChanged = true; }
    [[nodiscard]] Radians roll() const { return roll_; }
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
    void zoom(float offset) const;
};


#endif //CAMERA_H
