#ifndef MODEL_H
#define MODEL_H
#include <expected>
#include <vector>
#include <assimp/material.h>
#include <assimp/scene.h>
#include "engine/shader.h"

#include "mesh.h"

namespace Engine::Loader {
    class Model {
    public:
        explicit Model(const char *path);

        void Draw(const Shader &shader) const;

    private:
        std::vector<Mesh> meshes;
        std::vector<Texture> loaded_textures;
        std::string directory;

        void loadModel(const std::string &path);

        std::expected<void, std::string> processNode(const aiNode *node, const aiScene *scene);

        std::expected<Mesh, std::string> processMesh(const aiMesh *mesh, const aiScene *scene);

        std::vector<Texture> loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const std::string &typeName);
    };
}

#endif //MODEL_H
