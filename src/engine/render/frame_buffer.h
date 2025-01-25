#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H


class FrameBuffer {
private:
    unsigned int ID{};
    unsigned int width, height;

public:
    unsigned int ColorTextureID{};
    unsigned int DepthStencilTextureID{};

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
