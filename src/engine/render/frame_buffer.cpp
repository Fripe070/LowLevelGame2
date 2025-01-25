#include "frame_buffer.h"

#include <stdexcept>
#include <gl/glew.h>


unsigned int genColorTexture(const int width, const int height) {
    unsigned int colorTextureID;
    glGenTextures(1, &colorTextureID);
    glBindTexture(GL_TEXTURE_2D, colorTextureID);
    // When displaying the texture, it won't necessarily be the same size as the window, so we might need to interpolate
    // For example displaying the texture in a buffer preview window
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
    return colorTextureID;
}
unsigned int genDepthStencilTexture(const int width, const int height) {
    unsigned int depthStencilTextureID;
    glGenTextures(1, &depthStencilTextureID);
    glBindTexture(GL_TEXTURE_2D, depthStencilTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
    return depthStencilTextureID;
}

FrameBuffer::FrameBuffer(const int width, const int height): width(width), height(height) {
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    // Attach a color buffer to the framebuffer
    ColorTextureID = genColorTexture(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTextureID, 0);

    // Attach a depth and stencil buffer to the framebuffer
    DepthStencilTextureID = genDepthStencilTexture(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthStencilTextureID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Framebuffer is not complete!");
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &ID);
    glDeleteTextures(1, &ColorTextureID);
    glDeleteTextures(1, &DepthStencilTextureID);
}

void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void FrameBuffer::bind(const unsigned int target) const {
    glBindFramebuffer(target, ID);
}

void FrameBuffer::resize(const int width, const int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    // Using glTexStorage2D makes the textures immutable, so we need to delete and recreate them
    glDeleteTextures(1, &ColorTextureID);
    ColorTextureID = genColorTexture(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTextureID, 0);

    glDeleteTextures(1, &DepthStencilTextureID);
    DepthStencilTextureID = genDepthStencilTexture(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthStencilTextureID, 0);

    this->width = width;
    this->height = height;
}

FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept {
    if (this != &other) {
        glDeleteFramebuffers(1, &ID);
        glDeleteTextures(1, &ColorTextureID);
        glDeleteTextures(1, &DepthStencilTextureID);

        ID = other.ID;
        ColorTextureID = other.ColorTextureID;
        DepthStencilTextureID = other.DepthStencilTextureID;
        other.ID = 0;
        other.ColorTextureID = 0;
        other.DepthStencilTextureID = 0;
    }
    return *this;
}

FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept {
    ID = other.ID;
    ColorTextureID = other.ColorTextureID;
    DepthStencilTextureID = other.DepthStencilTextureID;
    other.ID = 0;
    other.ColorTextureID = 0;
    other.DepthStencilTextureID = 0;
}
