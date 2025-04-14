#pragma once
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "material.h"


namespace Resource {
    struct MeshVertex {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Normal = glm::vec3(0.0f);
        // We only support a single set of texture coordinates atm
        glm::vec2 TexCoords = glm::vec2(0.0f);
    };

    /*!
     * A mesh is a piece of geometry with a single material.
     * It manages its own OpenGL buffers.
     */
    class Mesh {
    public:
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        std::shared_ptr<PBRMaterial> material;

        unsigned int VAO{}, VBO{}, EBO{};
    public:
        Mesh() = default;
        ~Mesh();

        /*! Binds the mesh's VAO. */
        void bindBuffers() const;
        /*! (Re)creates the OpenGL buffers for this mesh based on its current data. */
        void rebuildGl();

        Expected<void> Draw(const glm::mat4& modelTransform) const;

        // Non-copyable
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        // Moveable
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
    };
}
