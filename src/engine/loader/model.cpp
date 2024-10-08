#include "model.h"

#include <engine/logging.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "texture.h"

namespace Engine::Loader {
    Model::Model(const char *path) {
        loadModel(path);
    }

    void Model::loadModel(const std::string &path) {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            logError("Assimp error: %s", import.GetErrorString());
            return; // TODO: Should we do more than just log? Throw? Expected?
        }
        directory = path.substr(0, path.find_last_of('/'));
        // TODO: Is there a better way to do this? We really need to sort out paths in general...

        auto result = processNode(scene->mRootNode, scene);
        if (!result.has_value()) {
            logError("Failed to process node: %s", result.error().c_str());
        }
    }

    std::expected<void, std::string> Model::processNode(const aiNode *node, const aiScene *scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            auto mesh = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
            if (!mesh.has_value())
                return std::unexpected("Failed to process mesh: " + mesh.error());
            meshes.push_back(mesh.value());
        }

        // Recursively process children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            auto result = processNode(node->mChildren[i], scene);
            if (!result.has_value())
                return std::unexpected("Failed to process node: " + result.error());
        }
        return {};
    }

#define UNPACK_VEC2(aiVec) {aiVec.x, aiVec.y}
#define UNPACK_VEC3(aiVec) {aiVec.x, aiVec.y, aiVec.z}

    std::expected<Mesh, std::string> Model::processMesh(const aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = UNPACK_VEC3(mesh->mVertices[i]);

            if (mesh->mNormals == nullptr)
                return std::unexpected("Mesh does not have normals");

            vertex.Normal = UNPACK_VEC3(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0] != nullptr)
                // A vertex can have up to 8 texture coordinates for whatever reason // TODO: Deal with this?
                    vertex.TexCoords = UNPACK_VEC2(mesh->mTextureCoords[0][i]);
            else
                vertex.TexCoords = {0.0f, 0.0f};
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, DIFFUSE_TEX_NAME);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, SPECULAR_TEX_NAME);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> Model::loadMaterialTextures(
        const aiMaterial *mat, const aiTextureType type, const std::string &typeName
    ) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString tPath;
            mat->GetTexture(type, i, &tPath);
            bool skip = false;
            for (const auto &texture: loaded_textures)
                if (texture.path != tPath.C_Str()) {
                    textures.push_back(texture);
                    skip = true;
                    break;
                }
            if (skip) continue;

            Texture texture;
            texture.id = loadTexture(directory + '/' + tPath.C_Str()).value();  // TODO: Path horror
            texture.type = typeName;
            texture.path = tPath.C_Str();
            textures.push_back(texture);
        }
        return textures;
    }

    void Model::Draw(const Shader &shader) const {
        for (auto mesh: meshes)
            mesh.Draw(shader);
    }
}
