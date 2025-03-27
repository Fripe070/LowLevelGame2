#pragma once
#include <engine/loader/shader/shader.h>

class ScreenOverlay {
  private:
    Engine::ShaderProgram shader;
    unsigned int VAO{}, VBO{}, EBO{};
  
  public:
    ScreenOverlay();
    ~ScreenOverlay();

    void draw(unsigned int texture) const;
  
    // Non-copyable
    ScreenOverlay(const ScreenOverlay&) = delete;
    ScreenOverlay& operator=(const ScreenOverlay&) = delete;
    // Moveable
    ScreenOverlay(ScreenOverlay&& other) noexcept;
    ScreenOverlay& operator=(ScreenOverlay&& other) noexcept;
};


