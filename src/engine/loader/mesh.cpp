#include "mesh.h"

#include <stdexcept>
#include <gl/glew.h>

namespace Engine::Loader {
    Mesh::Mesh(
        const std::vector<Vertex> &vertices,
        const std::vector<unsigned int> &indices,
        const std::vector<Texture> &textures
    ) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupGlMesh();
    }

    void Mesh::setupGlMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set all the properties of the vertices
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            sizeof(Vertex::Position) / sizeof(float), GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            nullptr); // first so offset is 0

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            sizeof(Vertex::Normal) / sizeof(float), GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void *>(offsetof(Vertex, Normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            sizeof(Vertex::TexCoords) / sizeof(float), GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void *>(offsetof(Vertex, TexCoords)));

        glBindVertexArray(0);  // Unbind VAO
    }

    void Mesh::Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const {
        // TODO: Flawed logic here makes textures "bleed over" to other materials if they don't have any.
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // Textures are offset by 1 each, if we overflow... well...
            std::string number;
            std::string name = textures[i].type;
            if (name == DIFFUSE_TEX_NAME)
                number = std::to_string(diffuseNr++);
            else if (name == SPECULAR_TEX_NAME)
                number = std::to_string(specularNr++);

            shader.setInt(("material." + name + number).c_str(), i); // material.texture_diffuse1

            const auto texture = textureManager.loadTexture(textures[i].path);
            if (!texture.has_value())
                throw std::runtime_error("Failed to load texture: " + textures[i].path);
            glBindTexture(GL_TEXTURE_2D, texture.value());
        }
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(GL_ZERO);
    }
}
