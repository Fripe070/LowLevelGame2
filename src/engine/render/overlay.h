#pragma once
#include "engine/loader/shader/graphics_shader.h"

class ScreenOverlay {
  private:
    Engine::GraphicsShader shader;
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


