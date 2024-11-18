#ifndef MODEL_H
#define MODEL_H
#include <expected>
#include <vector>
#include <assimp/material.h>
#include <assimp/scene.h>
#include "engine/loader/shader.h"
#include <glm/glm.hpp>

#include "mesh.h"

namespace Engine::Loader {
    // TODO: Bones
    // TODO: Animations
    // TODO: Sort by texture to try minimize texture swaps
    class Model {
    public:
        explicit Model(const std::string &path);

        void Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const;

    private:
        std::vector<Mesh> meshes;
        std::vector<Mesh::TextureRef> loaded_textures;
        std::string directory;

        std::expected<void, std::string> processNode(const aiNode *node, const aiScene *scene);

        std::expected<Mesh, std::string> processMesh(const aiMesh *mesh, const aiScene *scene);

        std::vector<Mesh::TextureRef> loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const std::string &typeName);
    };

    class NewModel {
    };
}

#endif //MODEL_H
