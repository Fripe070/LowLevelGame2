#include "mesh.h"

#include <engine/state.h>
#include <engine/resources/resource_manager.h>

namespace Resource {
    Mesh::~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void Mesh::rebuildGl() {
        // where the "re" in rebuild comes in :3 delete on 0 is no-op
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)),
            vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
            indices.data(), GL_STATIC_DRAW);

#define ENABLE_F_VERTEX_ATTRIB(index, member) \
        glEnableVertexAttribArray(index); \
        glVertexAttribPointer(index, \
            sizeof(MeshVertex::member) / sizeof(float), GL_FLOAT, \
            GL_FALSE, \
            sizeof(MeshVertex), \
            reinterpret_cast<void *>(offsetof(MeshVertex, member)))
        // Set all the properties of the vertices
        ENABLE_F_VERTEX_ATTRIB(0, Position);
        ENABLE_F_VERTEX_ATTRIB(1, Normal);
        ENABLE_F_VERTEX_ATTRIB(2, TexCoords);
#undef ENABLE_F_VERTEX_ATTRIB
    }

#pragma region Move Semantics
    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);

            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            material = std::move(other.material);

            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
        }
        return *this;
    }
    Mesh::Mesh(Mesh&& other) noexcept {
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        material = std::move(other.material);

        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
    }
#pragma endregion

    void Mesh::bindBuffers() const {
        glBindVertexArray(VAO);
    }

    Expected<void> Mesh::Draw(const glm::mat4& modelTransform) const {
        // TODO: POPULATE MATERIAL PROPERTIES INTO SHADER
        Engine::ResourceManager& resourceManager = engineState->resourceManager;
        const std::shared_ptr<Shader> shader = material->shader;
        if (!shader)
            return std::unexpected(ERROR("Mesh material has no shader"));
        shader->use();
        shader->setMat4("model", modelTransform);
        shader->setMat3("mTransposed", glm::mat3(glm::transpose(glm::inverse(modelTransform))));

        bindBuffers();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        return {};
    }

}
