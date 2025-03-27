#pragma once
#include <expected>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>

#include <engine/loader/shader/shader.h>
#include <engine/util/error.h>

// TODO: Bones and animations
// TODO: We should stop using paths as IDs... maybe pull a minecraft and use some sort of namespace path hybrid?

namespace Engine
{
    class ResourceManager;
}

namespace Engine::Loader {
    // TODO: Somehow couple materials and shaders. Perhaps materials should hold shared pointers to their shaders?
    //  Materials are just data associated with a shader. They don't necessarily need to be tied to Mesh objects in any way
    struct Material {
        std::string diffusePath;
        std::string specularPath;
        float shininess;

        std::expected<void, Error> PopulateShader(const ShaderProgram &shader, ResourceManager &resourceManager) const;
    };

    struct MeshVertex {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Normal = glm::vec3(0.0f);
        // Yes, texture coordinates can be 3D
        // We only support a single set of texture coordinates atm
        glm::vec2 TexCoords = glm::vec3(0.0f);
        // We only support a single vertex color atm
        glm::vec4 Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    };
    /*!
     * A mesh is a piece of geometry with a single material.
     * It manages its own OpenGL buffers.
     */
    class Mesh {
    public:

        // TODO: We don't technically need to store the vertices and indices in the mesh object, since they're in the OpenGL buffers
        //  The geometry will still need to be stored for the collision system, so it might be best to split the mesh into a data version and a renderable version?
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int materialIndex;

        // TODO: Moving vertices when constructing the mesh, only to move the entire mesh when making a scene seems a bit wasteful
        // A bit of a weird constructor, but moving in the vectors allows us to avoid copying them
        Mesh(
            std::vector<MeshVertex>&& vertices,
            std::vector<unsigned int>&& indices,
            unsigned int materialIndex
        );
        ~Mesh();

        // Non-copyable
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        // Moveable
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void bindGlMesh() const;

    private:
        unsigned int VAO{}, VBO{}, EBO{};
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

    class Scene {
    public:
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

        std::expected<void, Error> Draw(ResourceManager &resourceManager, const ShaderProgram &shader, const glm::mat4 &modelTransform) const;
    };

    // TODO: Document

    std::expected<Scene, Error> loadScene(const std::string &path);
    std::expected<Scene, Error> loadScene(const unsigned char* data, int size);
};


