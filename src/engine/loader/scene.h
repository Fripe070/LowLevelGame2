#ifndef SCENE_H
#define SCENE_H

#include <expected>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>

namespace Engine {
    class ShaderProgram;
    namespace Manager {
        class TextureManager;
    }
}


// TODO: Bones and animations
// TODO: We should stop using paths as IDs... maybe pull a minecraft and use some sort of namespace path hybrid?

namespace Engine::Loader {
    struct Material {
        std::string diffusePath;
        std::string specularPath;
        float shininess;

        std::expected<void, std::string> PopulateShader(const ShaderProgram &shader, Manager::TextureManager &textureManager) const;
    };

    /*!
     * A mesh is a piece of geometry with a single material.
     * It manages its own OpenGL buffers.
     */
    class Mesh {
    public:
        struct Vertex {
            glm::vec3 Position;
            glm::vec3 Normal;
            // Yes, texture coordinates can be 3D
            // We only support a single set of texture coordinates atm
            glm::vec3 TexCoords;
            // We only support a single vertex color atm
            glm::vec4 Color;
        };

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int materialIndex;

        Mesh(
            const std::vector<Vertex> &vertices,
            const std::vector<unsigned int> &indices,
            unsigned int materialIndex
        );
        ~Mesh();

        void LoadGlMesh() const;

    private:
        unsigned int VAO, VBO, EBO;
        /*!
         * Sets up the OpenGL buffers for this mesh.
         * @note Leaves the VAO bound.
         */
        void setupGlMesh();
    };

    struct Node {
        std::vector<Node> children;
        glm::mat4x4 transform;

        std::vector<unsigned int> meshIndices;
    };

    struct Scene {
        Node rootNode;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;

        std::expected<void, std::string> Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const;
    };

    std::expected<Scene, std::string> loadScene(const std::string &path);
};


#endif
