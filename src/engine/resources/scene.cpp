#include "scene.h"

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/state.h>

#include "engine/resources/mesh.h"

// TODO: Put more consideration into this depending on our needs (for example mesh sorting?)
//  This is just a super simple first pass set of flags where much consideration hasn't been put in
constexpr auto ASSIMP_FLAGS = (
    aiProcess_Triangulate
    | aiProcess_FlipUVs
    | aiProcess_OptimizeMeshes
    | aiProcess_OptimizeGraph
    );


#define UNPACK_MAT4(aiMat) { \
    aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4, \
    aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4, \
    aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4, \
    aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4 \
}
#define UNPACK_VEC2(aiVec) {aiVec.x, aiVec.y}
#define UNPACK_VEC3(aiVec) {aiVec.x, aiVec.y, aiVec.z}
#define UNPACK_VEC4(aiVec) {aiVec.x, aiVec.y, aiVec.z, aiVec.w}
#define UNPACK_RGBA(aiColor) {aiColor.r, aiColor.g, aiColor.b, aiColor.a}

namespace Resource
{
    Expected<void> Scene::DrawNode(const Node& node, const glm::mat4& parentTransform) const { // NOLINT(*-no-recursion)
        const glm::mat4 transform = parentTransform * node.transform;
        for (const unsigned int meshIndex : node.meshIndices) {
            if (meshIndex >= meshes.size())
                return std::unexpected(ERROR("Mesh index out of bounds"));
            Expected<void> result = meshes[meshIndex].Draw(transform);
            if (!result.has_value())
                return std::unexpected(FW_ERROR(result.error(), "Failed to draw mesh"));
        }

        for (const Node& child : node.children) {
            Expected<void> result = DrawNode(child, transform);
            if (!result.has_value())
                return std::unexpected(FW_ERROR(result.error(), "Failed to draw child node"));
        }

        return {};
    }

    Expected<void> Scene::Draw(const glm::mat4& transform) const {
        return DrawNode(root, transform);
    }
}

namespace Resource::Loading {
    Expected<Scene> loadScene(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene* loadedScene = importer.ReadFile(path.c_str(), ASSIMP_FLAGS);
        if (!loadedScene || loadedScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !loadedScene->mRootNode)
            return std::unexpected(ERROR(std::string("Failed to load scene file: ") + importer.GetErrorString()));

        Expected<Scene> scene = loadScene(*loadedScene);
        if (!scene.has_value())
            return std::unexpected(FW_ERROR(scene.error(), "Failed to load scene from file"));
        return scene;
    }
    Expected<Scene> loadScene(const unsigned char* data, const int size)
    {
        Assimp::Importer importer;
        const aiScene* loadedScene = importer.ReadFileFromMemory(data, size, ASSIMP_FLAGS);
        if (!loadedScene || loadedScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !loadedScene->mRootNode)
            return std::unexpected(ERROR(std::string("Failed to load scene data: ") + importer.GetErrorString()));

        Expected<Scene> scene = loadScene(*loadedScene);
        if (!scene.has_value())
            return std::unexpected(FW_ERROR(scene.error(), "Failed to load scene from data"));
        return scene;
    }

    Expected<Node> processNode(const aiNode* loadedNode);
    Expected<PBRMaterial> processMaterial(const aiMaterial* loadedMaterial);
    Expected<Mesh> processMesh(const aiMesh* loadedMesh, const std::shared_ptr<PBRMaterial>& material);

    Expected<Scene> loadScene(const aiScene &scene) {
        Scene resultScene;
        // Materials
        resultScene.materials.reserve(scene.mNumMaterials);
        for (unsigned int i = 0; i < scene.mNumMaterials; i++) {
            Expected<PBRMaterial> material = processMaterial(scene.mMaterials[i]);
            if (!material.has_value())
                return std::unexpected(FW_ERROR(material.error(), "Failed to load material "+std::to_string(i)));
            resultScene.materials.push_back(std::move(material.value()));
        }
        // Meshes
        resultScene.meshes.reserve(scene.mNumMeshes);
        for (unsigned int i = 0; i < scene.mNumMeshes; i++) {
            unsigned int matIndex = scene.mMeshes[i]->mMaterialIndex;
            if (matIndex >= resultScene.materials.size())
                return std::unexpected(ERROR(
                    "Encountered invalid mesh material index " + std::to_string(scene.mMeshes[i]->mMaterialIndex)));
            Expected<Mesh> mesh = processMesh(
                scene.mMeshes[i],
                std::make_shared<PBRMaterial>(resultScene.materials[matIndex])
            );
            if (!mesh.has_value())
                return std::unexpected(FW_ERROR(mesh.error(), "Failed to load mesh "+std::to_string(i)));
            resultScene.meshes.push_back(std::move(mesh.value()));
        }
        // Nodes
        auto rootNode = processNode(scene.mRootNode);
        if (!rootNode.has_value())
            return std::unexpected(FW_ERROR(rootNode.error(), "Failed to load scene root node"));
        resultScene.root = rootNode.value();

        return resultScene;
    }

    Expected<Node> processNode(const aiNode *loadedNode) { // NOLINT(*-no-recursion)
        Node resultNode;
        resultNode.transform = UNPACK_MAT4(loadedNode->mTransformation);

        resultNode.meshIndices.reserve(loadedNode->mNumMeshes);
        for (unsigned int i = 0; i < loadedNode->mNumMeshes; i++) {
            resultNode.meshIndices.push_back(loadedNode->mMeshes[i]);
        }

        resultNode.children.reserve(loadedNode->mNumChildren);
        for (unsigned int i = 0; i < loadedNode->mNumChildren; i++) {
            Expected<Node> result = processNode(loadedNode->mChildren[i]);
            if (!result.has_value())
                return std::unexpected(FW_ERROR(result.error(), "Failed to load child node " + std::to_string(i)));
            resultNode.children.push_back(std::move(result.value()));
        }

        return resultNode;
    }

    Expected<PBRMaterial> processMaterial(const aiMaterial *loadedMaterial) {
        PBRMaterial resultMaterial{
            // TODO: Don't hardcode this
            .shader = engineState->resourceManager.loadShader(
                "resources/assets/shaders/vert.vert",
                "resources/assets/shaders/frag.frag"
            )
        };
        SPDLOG_TRACE("Loading material \"{}\"", loadedMaterial->GetName().C_Str());

        aiString path;
        if (loadedMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
            resultMaterial.albedo = engineState->resourceManager.loadTexture(path.C_Str());
        }
        if (loadedMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
            resultMaterial.normal = engineState->resourceManager.loadTexture(path.C_Str());
        }
        if (loadedMaterial->GetTexture(aiTextureType_SHININESS, 0, &path) == AI_SUCCESS) {
            resultMaterial.roughness = engineState->resourceManager.loadTexture(path.C_Str());
        }
        if (loadedMaterial->GetTexture(aiTextureType_REFLECTION, 0, &path) == AI_SUCCESS) {
            resultMaterial.metallic = engineState->resourceManager.loadTexture(path.C_Str());
        }
        if (loadedMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path) == AI_SUCCESS) {
            resultMaterial.ambientOcclusion = engineState->resourceManager.loadTexture(path.C_Str());
        }

        return resultMaterial;
    }

    Expected<Mesh> processMesh(const aiMesh *loadedMesh, const std::shared_ptr<PBRMaterial>& material) {
        Mesh resultMesh;
        resultMesh.material = material;

        resultMesh.vertices.reserve(loadedMesh->mNumVertices);
        for (unsigned int i = 0; i < loadedMesh->mNumVertices; i++) {
            MeshVertex vertex{};
            vertex.Position = UNPACK_VEC3(loadedMesh->mVertices[i]);
            vertex.Normal = UNPACK_VEC3(loadedMesh->mNormals[i]);
            if (loadedMesh->mTextureCoords[0]) {  // We only support a single set of texture coordinates atm
                vertex.TexCoords = UNPACK_VEC2(loadedMesh->mTextureCoords[0][i]);
            }

            resultMesh.vertices.push_back(vertex);
        }
        resultMesh.indices.reserve(loadedMesh->mNumFaces * 3); // 3 indices per face, assuming all faces are triangles
        for (unsigned int i = 0; i < loadedMesh->mNumFaces; i++) {
            const aiFace &face = loadedMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                resultMesh.indices.push_back(face.mIndices[j]);
        }

        resultMesh.rebuildGl();

        return resultMesh;
    }
}
