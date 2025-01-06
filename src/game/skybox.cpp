#include "skybox.h"

#include <array>
#include <engine/loader/shader/graphics_shader.h>
#include <gl/glew.h>
#include <engine/util/geometry.h>



Skybox::Skybox() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    constexpr auto skyboxVertPos = getCubeVertices(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertPos), skyboxVertPos.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndicesInside), CubeIndicesInside.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

Skybox::~Skybox() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
Skybox &Skybox::operator=(Skybox &&other) noexcept {
    if (this != &other) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        BUFFERS_MV_FROM_TO(other, this);
    }
    return *this;
}
Skybox::Skybox(Skybox &&other) noexcept {
    BUFFERS_MV_FROM_TO(other, this);
}

std::expected<void, std::string> Skybox::draw(const unsigned int cubemap, const Engine::GraphicsShader &shader) const {
    glDepthFunc(GL_LEQUAL);

    shader.setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sizeof(CubeIndicesInside) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

    glDepthFunc(GL_LESS);
    return {};
}
