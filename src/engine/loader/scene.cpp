#include "scene.h"

#include <iostream>
#include <assimp/cimport.h>
#include <engine/logging.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/ext/matrix_transform.hpp>

#include "shader.h"
#include "engine/manager/texture.h"

#ifndef NDEBUG
#include <chrono>
#endif


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


namespace Engine::Loader {
#pragma region Loading
    std::expected<Node, std::string> processNode(const aiNode *loadedNode);
    std::expected<Mesh, std::string> processMesh(const aiMesh *loadedMesh);
    std::expected<Material, std::string> processMaterial(const aiMaterial *loadedMaterial);

    std::expected<Scene, std::string> loadScene(const std::string &path) {
#ifndef NDEBUG
        const auto start = std::chrono::high_resolution_clock::now();
#endif
        Assimp::Importer importer;
        const aiScene* loadedNode = importer.ReadFile(path.c_str(),
            aiProcess_Triangulate
            | aiProcess_FlipUVs
            // TODO: Add more post processing flags if needed
        );
        if (!loadedNode || loadedNode->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !loadedNode->mRootNode)
            return std::unexpected(std::string("Failed to load scene: ") + importer.GetErrorString());

        if (loadedNode->mNumTextures > 0)
            logWarn("Embedded textures are not supported");
        if (loadedNode->mNumLights > 0)
            logWarn("Lights are not supported");
        if (loadedNode->mNumCameras > 0)
            logWarn("Cameras are not supported");
        if (loadedNode->mNumAnimations > 0)
            logWarn("Animations are not supported");

        // Load the node tree
        std::expected<Node, std::string> rootNode = processNode(loadedNode->mRootNode);
        if (!rootNode.has_value())
            return std::unexpected(rootNode.error());

        std::vector<Mesh> meshes;
        std::vector<Material> materials;

        // Load all the meshes
        meshes.reserve(loadedNode->mNumMeshes);
        for (unsigned int i = 0; i < loadedNode->mNumMeshes; i++) {
            std::expected<Mesh, std::string> mesh = processMesh(loadedNode->mMeshes[i]);
            if (!mesh.has_value())
                return std::unexpected(mesh.error());
            meshes.push_back(std::move(mesh.value()));
        }

        // Load all the materials
        materials.reserve(loadedNode->mNumMaterials);
        for (unsigned int i = 0; i < loadedNode->mNumMaterials; i++) {
            std::expected<Material, std::string> material = processMaterial(loadedNode->mMaterials[i]);
            if (!material.has_value())
                return std::unexpected(material.error());
            materials.push_back(material.value());
        }

#ifndef NDEBUG
        logDebug("Loaded scene \"%s\" in %d ms", path.c_str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());
#else
        logDebug("Loaded scene \"%s\"", path.c_str());
#endif

        return Scene{
            rootNode.value(),
            std::move(meshes),
            materials,
        };
    }

    std::expected<Node, std::string> processNode(const aiNode *loadedNode) {
        Node resultNode;
        resultNode.transform = UNPACK_MAT4(loadedNode->mTransformation);

        resultNode.children.reserve(loadedNode->mNumChildren);
        for (unsigned int i = 0; i < loadedNode->mNumChildren; i++) {
            std::expected<Node, std::string> result = processNode(loadedNode->mChildren[i]);
            if (!result.has_value())
                return std::unexpected(result.error());
            resultNode.children.push_back(result.value());
        }

        return resultNode;
    }

    std::expected<Mesh, std::string> processMesh(const aiMesh *loadedMesh) {
    // std::expected<void, std::string> processMesh(const aiMesh *loadedMesh, std::vector<Mesh::Vertex> &vertices, std::vector<unsigned int> &indices, unsigned int &materialIndex) {
        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        const unsigned int materialIndex = loadedMesh->mMaterialIndex;
        // materialIndex = loadedMesh->mMaterialIndex;

        for (unsigned int i = 0; i < loadedMesh->mNumVertices; i++) {
            Mesh::Vertex vertex;
            vertex.Position = UNPACK_VEC3(loadedMesh->mVertices[i]);

            if (loadedMesh->HasNormals())
                vertex.Normal = UNPACK_VEC3(loadedMesh->mNormals[i]);

            // 0 since we only support a single set of texture coordinates atm (the first one)
            if (loadedMesh->HasTextureCoords(0))
                vertex.TexCoords = UNPACK_VEC2(loadedMesh->mTextureCoords[0][i]);

            // 0 since we only support a single vertex color atm (the first one)
            if (loadedMesh->HasVertexColors(0))
                vertex.Color = UNPACK_RGBA(loadedMesh->mColors[0][i]);

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < loadedMesh->mNumFaces; i++) {
            const aiFace &face = loadedMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh{std::move(vertices), std::move(indices), materialIndex};
    }

    std::expected<Material, std::string> processMaterial(const aiMaterial *loadedMaterial) {
        Material resultMaterial;

        aiString path;
        if (loadedMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            resultMaterial.diffusePath = path.C_Str();
        else
            logWarn("Material %s does not have a diffuse texture", loadedMaterial->GetName().C_Str());
        if (loadedMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
            resultMaterial.specularPath = path.C_Str();
        else
            logWarn("Material %s does not have a specular texture", loadedMaterial->GetName().C_Str());

        // TODO: This does not make sense in the context of our renderer
        if (loadedMaterial->Get(AI_MATKEY_REFLECTIVITY, resultMaterial.shininess) != AI_SUCCESS) {
            logWarn("Material %s does not have shininess", loadedMaterial->GetName().C_Str());
            resultMaterial.shininess = 32.0f;
        }

        return resultMaterial;
    }
#pragma endregion


#pragma region Scene Rendering
    void Mesh::BindGlMesh() const {
        glBindVertexArray(VAO);
    }

    std::expected<void, std::string> Scene::Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader, const glm::mat4 &modelTransform) const {
        shader.use();
        for (const Mesh &mesh: meshes) {
            auto matRet = materials[mesh.materialIndex].PopulateShader(shader, textureManager);
            if (!matRet.has_value())
                return std::unexpected(matRet.error());

            const auto model = rootNode.transform * modelTransform;
            shader.setMat4("model", model);
            shader.setMat3("mTransposed", glm::mat3(glm::transpose(glm::inverse(model))));

            mesh.BindGlMesh();
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        return {};
    }

    std::expected<void, std::string> Material::PopulateShader(const ShaderProgram &shader, Manager::TextureManager &textureManager) const {
        shader.setFloat("material.shininess", shininess);
        shader.setInt("material.test", textureManager.errorTexture);

        std::expected<unsigned int, std::string> diffuse = textureManager.getTexture(diffusePath);
        if (!diffuse.has_value())
            return std::unexpected(diffuse.error());
        shader.setInt("material.texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse.value_or(textureManager.errorTexture));

        std::expected<unsigned int, std::string> specular = textureManager.getTexture(specularPath);
        if (!specular.has_value())
            return std::unexpected(specular.error());
        shader.setInt("material.texture_specular", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular.value_or(textureManager.errorTexture));

        return {};
    }
#pragma endregion


#pragma region Constructing & memory safety stuff
    Scene::Scene(
        const Node &rootNode,
        std::vector<Mesh> &&meshes,
        const std::vector<Material> &materials
    ) noexcept : rootNode(rootNode), meshes(std::move(meshes)), materials(materials) {}

    Scene::Scene(Scene &&other) noexcept {
        rootNode = std::move(other.rootNode);
        meshes = std::move(other.meshes);
        materials = std::move(other.materials);
    }
    Scene &Scene::operator=(Scene &&other) noexcept {
        if (this != &other) {
            rootNode = std::move(other.rootNode);
            meshes = std::move(other.meshes);
            materials = std::move(other.materials);
        }
        return *this;
    }

    Mesh::Mesh(
        std::vector<Vertex> &&vertices,
        std::vector<unsigned int> &&indices,
        const unsigned int materialIndex
    ) : vertices(std::move(vertices)), indices(std::move(indices)), materialIndex(materialIndex) {
        setupGlMesh();
    }

    Mesh::~Mesh() {
        if (VAO == 0 && VBO == 0 && EBO == 0)
            return;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    Mesh::Mesh(Mesh &&other) noexcept {
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        materialIndex = other.materialIndex;

        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
    }
    Mesh &Mesh::operator=(Mesh &&other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);

            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            materialIndex = other.materialIndex;

            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
        }
        return *this;
    }

    void Mesh::setupGlMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set all the properties of the vertices
#define ENABLE_F_VERTEX_ATTRIB(index, member) \
        glEnableVertexAttribArray(index); \
        glVertexAttribPointer(index, \
            sizeof(Vertex::member) / sizeof(float), GL_FLOAT, \
            GL_FALSE, \
            sizeof(Vertex), \
            reinterpret_cast<void *>(offsetof(Vertex, member)))

        ENABLE_F_VERTEX_ATTRIB(0, Position);
        ENABLE_F_VERTEX_ATTRIB(1, Normal);
        ENABLE_F_VERTEX_ATTRIB(2, TexCoords);
        ENABLE_F_VERTEX_ATTRIB(3, Color);
#undef ENABLE_F_VERTEX_ATTRIB
    }
#pragma endregion
};

