#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H


class FrameBuffer {
private:
    unsigned int ID{};

public:
    unsigned int ColorTextureID{};
    unsigned int DepthStencilRenderBufferID{};

    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void resize(int width, int height) const;

    void bind() const;

    // Non-copyable
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    // Moveable
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
};


#endif
