#include "frame_buffer.h"

#include <stdexcept>
#include <gl/glew.h>


FrameBuffer::FrameBuffer(int width, int height) {
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
#pragma region "Color texture"
    glGenTextures(1, &ColorTextureID);
    glBindTexture(GL_TEXTURE_2D, ColorTextureID);
    // We won't necessarily render to a texture the same size as the window, so we might need to interpolate
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach to our framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTextureID, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#pragma endregion
#pragma region "Depth and stencil texture"
    glGenTextures(1, &DepthStencilTextureID);
    glBindTexture(GL_TEXTURE_2D, DepthStencilTextureID);
    // We won't necessarily render to a texture the same size as the window, so we might need to interpolate
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach to our framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthStencilTextureID, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
#pragma endregion

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
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

void FrameBuffer::resize(const int width, const int height) const {
    glBindTexture(GL_TEXTURE_2D, ColorTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, DepthStencilTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
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
