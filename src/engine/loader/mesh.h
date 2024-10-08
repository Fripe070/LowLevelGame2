#ifndef MESH_H
#define MESH_H
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "engine/shader.h"

#define DIFFUSE_TEX_NAME "texture_diffuse"
#define SPECULAR_TEX_NAME "texture_specular"

namespace Engine::Loader {
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Texture {
        unsigned int id;
        std::string type;
        std::string path;
    };

    class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices,
             const std::vector<Texture> &textures);

        void Draw(const Shader &shader) const;

    private:
        unsigned int VAO, VBO, EBO;

        void setupGlMesh();
    };
}

#endif //MESH_H
