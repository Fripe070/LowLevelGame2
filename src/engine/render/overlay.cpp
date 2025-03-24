#include "overlay.h"

#include "engine/util/geometry.h"


ScreenOverlay::ScreenOverlay() : shader("resources/assets/shaders/overlay.vert", "resources/assets/shaders/overlay.frag") {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenSpaceQuadVertices), ScreenSpaceQuadVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ScreenSpaceQuadIndices), ScreenSpaceQuadIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

void ScreenOverlay::draw(const unsigned int texture) const {
    shader.use();
    shader.setInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sizeof(ScreenSpaceQuadIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);
}

ScreenOverlay::~ScreenOverlay() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
ScreenOverlay &ScreenOverlay::operator=(ScreenOverlay &&other) noexcept {
    if (this != &other) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        BUFFERS_MV_FROM_TO(other, this);
        shader = std::move(other.shader);
    }
    return *this;
}
ScreenOverlay::ScreenOverlay(ScreenOverlay &&other) noexcept: shader(std::move(other.shader)) {
    BUFFERS_MV_FROM_TO(other, this);
}
