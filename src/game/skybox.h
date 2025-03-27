#pragma once

namespace Engine {
    class ShaderProgram;
    namespace Manager {
        class TextureManager;
    }
}


class Skybox {
public:
    Skybox();
    ~Skybox();
    void draw(unsigned int cubemap, const Engine::ShaderProgram &shader) const;

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

