#pragma once
#include <engine/resources/shader.h>

class Skybox {
public:
    Skybox();
    ~Skybox();
    // TODO: Terrible. Don't pass the shader around like this, instead access it from the resource manager
    void draw(unsigned int cubemap, const Resource::Shader &shader) const;

private:
    unsigned int VAO{}, VBO{}, EBO{};

public:
    // Non-copyable
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;
    // Moveable
    Skybox(Skybox&& other) noexcept;
    Skybox& operator=(Skybox&& other) noexcept;
};

