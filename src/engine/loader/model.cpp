#include "model.h"

#include <stdexcept>
#include <engine/logging.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"

namespace Engine::Loader {
    Model::Model(const std::string &path) {
        // TODO: Is there a better way to do this? We really need to sort out paths in general...
        directory = path.substr(0, path.find_last_of('/'));

        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            throw std::runtime_error(std::string("Failed to load model: ") + import.GetErrorString());

        auto result = processNode(scene->mRootNode, scene);
        if (!result.has_value())
            throw std::runtime_error("Failed to process node: " + result.error());

        logDebug("Loaded model \"%s\" with %d meshes", path.c_str(), meshes.size());
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
        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Mesh::TextureRef> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Mesh::Vertex vertex;
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
        std::vector<Mesh::TextureRef> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, DIFFUSE_TEX_NAME);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Mesh::TextureRef> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, SPECULAR_TEX_NAME);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        logDebug("Loaded mesh with %d vertices", vertices.size(), indices.size());
        return Mesh(vertices, indices, textures);
    }

    std::vector<Mesh::TextureRef> Model::loadMaterialTextures(
        const aiMaterial *mat, const aiTextureType type, const std::string &typeName
    ) {
        std::vector<Mesh::TextureRef> textures;
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

            Mesh::TextureRef texture;
            texture.type = typeName;
            texture.path = directory + '/' + tPath.C_Str();  // Will be loaded on first draw call using it
            textures.push_back(texture);
        }
        return textures;
    }

    void Model::Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const {
        for (auto mesh: meshes)
            mesh.Draw(textureManager, shader);
    }
}
