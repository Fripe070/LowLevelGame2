#pragma once
#include <engine/resources/shader.h>

#include "engine/resources/texture.h"

class Skybox {
public:
    std::shared_ptr<Resource::ManagedTexture> cubemap;
    std::shared_ptr<Resource::Shader> shader;

    Skybox();
    Skybox(const std::shared_ptr<Resource::ManagedTexture>& cubemap);
    ~Skybox();

    void draw() const;

private:
    unsigned int VAO{}, VBO{}, EBO{};
    void initGL();
public:
    // Non-copyable
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;
    // Moveable
    Skybox(Skybox&& other) noexcept;
    Skybox& operator=(Skybox&& other) noexcept;
};

