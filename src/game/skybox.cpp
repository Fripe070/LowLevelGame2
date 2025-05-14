#include "skybox.h"

#include <array>
#include <GL/glew.h>

#include "engine/state.h"
#include "engine/util/geometry.h"


void Skybox::draw() const
{
    glDepthFunc(GL_GEQUAL);

    shader->use();
    shader->setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->textureID);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sizeof(CubeIndicesInside) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

    glDepthFunc(GL_GREATER);  // Our default depth function
}

void Skybox::initGL() {
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

Skybox::Skybox()
{
    shader = engineState->resourceManager.loadShader(
        "resources/assets/shaders/sb_vert.vert", "resources/assets/shaders/sb_frag.frag");
    this->cubemap = engineState->resourceManager.errorCubemap;
    initGL();
}

Skybox::Skybox(const std::shared_ptr<Resource::ManagedTexture>& cubemap)
{
    shader = engineState->resourceManager.loadShader(
        "resources/assets/shaders/sb_vert.vert", "resources/assets/shaders/sb_frag.frag");
    this->cubemap = cubemap;
    initGL();
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
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        this->EBO = other.EBO;
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
    }
    return *this;
}
Skybox::Skybox(Skybox &&other) noexcept {
    this->VAO = other.VAO;
    this->VBO = other.VBO;
    this->EBO = other.EBO;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
}