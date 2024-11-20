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
            glm::vec2 TexCoords;
            // We only support a single vertex color atm
            glm::vec4 Color;
        };

        // TODO: We don't technically need to store the vertices and indices in the mesh object, since they're in the OpenGL buffers
        //  The geometry will still need to be stored for the collision system, so it might be best to split the mesh into a data version and a renderable version?
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int materialIndex;

        // TODO: Moving vertices when constructing the mesh, only to move the entire mesh when making a scene seems a bit wasteful
        // A bit of a weird constructor, but moving in the vectors allows us to avoid copying them'
        Mesh(
            std::vector<Vertex>&& vertices,
            std::vector<unsigned int>&& indices,
            const unsigned int materialIndex
        );
        ~Mesh();

        // Non-copyable
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        // Moveable
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void BindGlMesh() const;

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

        Scene(
            const Node &rootNode,
            std::vector<Mesh>&& meshes,
            const std::vector<Material> &materials
        ) noexcept;

        // Non-copyable
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        // Moveable
        Scene(Scene&& other) noexcept;
        Scene& operator=(Scene&& other) noexcept;

        std::expected<void, std::string> Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader, const glm::mat4 &modelTransform) const;
    };

    std::expected<Scene, std::string> loadScene(const std::string &path);
};


#endif
