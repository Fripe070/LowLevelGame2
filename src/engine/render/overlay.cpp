#include "overlay.h"

#include <engine/state.h>
#include <GL/glew.h>

#include "engine/util/geometry.h"


ScreenOverlay::ScreenOverlay() {
    shader = engineState->resourceManager.loadShader({
        {Resource::ShaderType::VERTEX, "resources/assets/shaders/overlay.vert"},
        {Resource::ShaderType::FRAGMENT, "resources/assets/shaders/overlay.frag"}
    });

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenSpaceQuadVertices), ScreenSpaceQuadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

void ScreenOverlay::draw(const unsigned int texture) const {
    shader->use();
    shader->setInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

ScreenOverlay::~ScreenOverlay() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
ScreenOverlay &ScreenOverlay::operator=(ScreenOverlay &&other) noexcept {
    if (this != &other) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        other.VAO = 0;
        other.VBO = 0;
        shader = std::move(other.shader);
    }
    return *this;
}
ScreenOverlay::ScreenOverlay(ScreenOverlay &&other) noexcept: shader(std::move(other.shader)) {
    this->VAO = other.VAO;
    this->VBO = other.VBO;
    other.VAO = 0;
    other.VBO = 0;
}
