#ifndef MESH_H
#define MESH_H
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "engine/loader/shader.h"
#include <engine/managers.h>

#define DIFFUSE_TEX_NAME "texture_diffuse"
#define SPECULAR_TEX_NAME "texture_specular"

namespace Engine::Loader {
    class Mesh {
    public:
        struct Vertex {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoords;
        };
        struct Texture {
            std::string type;
            std::string path;
        };

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(
            const std::vector<Vertex> &vertices,
            const std::vector<unsigned int> &indices,
            const std::vector<Texture> &textures
        );

        void Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const;

    private:
        unsigned int VAO, VBO, EBO;

        void setupGlMesh();
    };
}

#endif //MESH_H
