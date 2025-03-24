#pragma once
#include <expected>
#include <string>

#include "engine/loader/shader/graphics_shader.h"


namespace Engine {
    class GraphicsShader;
    namespace Manager {
        class TextureManager;
    }
}


class Skybox {
public:
    Skybox();
    ~Skybox();
    std::expected<void, std::string> draw(unsigned int cubemap, const Engine::GraphicsShader &shader) const;

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

