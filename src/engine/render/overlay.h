#pragma once
#include <engine/resources/shader.h>

class ScreenOverlay {
  private:
    std::shared_ptr<Resource::Shader> shader;
    unsigned int VAO{}, VBO{};
  
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


