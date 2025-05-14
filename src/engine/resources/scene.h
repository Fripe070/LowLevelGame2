#pragma once
#include <expected>
#include <valarray>
#include <vector>
#include <glm/mat4x4.hpp>

#include "engine/resources/material.h"
#include "engine/resources/mesh.h"
#include "engine/util/error.h"


struct aiScene;

namespace Resource
{
    struct Node {
        std::vector<Node> children;
        glm::mat4x4 transform;

        std::vector<unsigned int> meshIndices;
    };

    class Scene {
    public:
        Node root = {};
        std::vector<Mesh> meshes;
        std::vector<PBRMaterial> materials;

        Expected<void> Draw(const glm::mat4& transform = glm::mat4(1.0)) const;

    private:
        Expected<void> DrawNode(const Node& node, const glm::mat4& parentTransform) const;

    public:
        // // Non-copyable
        // Scene(const Scene&) = delete;
        // Scene& operator=(const Scene&) = delete;
        // // Moveable
        // Scene(Scene&& other) noexcept;
        // Scene& operator=(Scene&& other) noexcept;
    };
}

namespace Resource::Loading
{
    [[nodiscard]] Expected<Scene> loadScene(const std::string& path);
    [[nodiscard]] Expected<Scene> loadScene(const unsigned char* data, int size);
    [[nodiscard]] Expected<Scene> loadScene(const aiScene& scene);
}
