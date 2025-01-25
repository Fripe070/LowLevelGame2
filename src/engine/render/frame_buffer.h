#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include <engine/run.h>


class FrameBuffer {
private:
    unsigned int ID{};
    int width{}, height{};

public:
    unsigned int ColorTextureID{};
    unsigned int DepthStencilTextureID{};

    [[nodiscard]] WindowSize getSize() const { return {width, height}; }

    FrameBuffer(int width, int height);
    ~FrameBuffer();

    /*!
     * @note Will bind the framebuffer.
     * @note Will delete the current framebuffer textures.
     */
    void resize(int width, int height);

    void bind() const;
    void bind(unsigned int target) const;

    // Non-copyable
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    // Moveable
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
};


#endif
