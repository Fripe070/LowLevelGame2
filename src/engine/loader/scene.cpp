#include "scene.h"

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/loader/shader/shader.h>
#include <engine/util/geometry.h>
#include <glm/ext/matrix_transform.hpp>

#include "engine/resources/resource_manager.h"
#include "engine/util/logging.h"


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
    std::expected<Node, Error> processNode(const aiNode *loadedNode);
    std::expected<Mesh, Error> processMesh(const aiMesh *loadedMesh);
    std::expected<Material, Error> processMaterial(const aiMaterial *loadedMaterial);
    std::expected<Scene, Error> loadScene(const aiScene* loadedNode);

    constexpr auto ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_FlipUVs;

    std::expected<Scene, Error> loadScene(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene* loadedNode = importer.ReadFile(path.c_str(), ASSIMP_FLAGS);
        if (!loadedNode || loadedNode->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !loadedNode->mRootNode)
            return std::unexpected(ERROR(std::string("Failed to load scene: ") + importer.GetErrorString()));
        return loadScene(loadedNode);
    }
    std::expected<Scene, Error> loadScene(const unsigned char* data, const int size)
    {
        Assimp::Importer importer;
        const aiScene* loadedNode = importer.ReadFileFromMemory(data, size, ASSIMP_FLAGS);
        if (!loadedNode || loadedNode->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !loadedNode->mRootNode)
            return std::unexpected(ERROR(std::string("Failed to load scene: ") + importer.GetErrorString()));
        return loadScene(loadedNode);
    }

    std::expected<Scene, Error> loadScene(const aiScene* loadedNode) {
        if (loadedNode->mNumTextures > 0)
            SPDLOG_WARN("Embedded textures are not supported");
        if (loadedNode->mNumLights > 0)
            SPDLOG_WARN("Lights are not supported");
        if (loadedNode->mNumCameras > 0)
            SPDLOG_WARN("Cameras are not supported");
        if (loadedNode->mNumAnimations > 0)
            SPDLOG_WARN("Animations are not supported");

        // Load the node tree
        std::expected<Node, Error> rootNode = processNode(loadedNode->mRootNode);
        if (!rootNode.has_value())
            return std::unexpected(FW_ERROR(rootNode.error(), "Failed to load node tree"));

        std::vector<Mesh> meshes;
        std::vector<Material> materials;

        // Load all the meshes
        meshes.reserve(loadedNode->mNumMeshes);
        for (unsigned int i = 0; i < loadedNode->mNumMeshes; i++) {
            std::expected<Mesh, Error> mesh = processMesh(loadedNode->mMeshes[i]);
            if (!mesh.has_value())
                return std::unexpected(FW_ERROR(mesh.error(), "Failed to load mesh "+std::to_string(i)));
            meshes.push_back(std::move(mesh.value()));
        }

        // Load all the materials
        materials.reserve(loadedNode->mNumMaterials);
        for (unsigned int i = 0; i < loadedNode->mNumMaterials; i++) {
            std::expected<Material, Error> material = processMaterial(loadedNode->mMaterials[i]);
            if (!material.has_value())
                return std::unexpected(FW_ERROR(material.error(), "Failed to load material "+std::to_string(i)));
            materials.push_back(material.value());
        }
        SPDLOG_DEBUG("Loaded scene \"%s\"", path.c_str());

        return Scene{
            rootNode.value(),
            std::move(meshes),
            materials,
        };
    }

    std::expected<Node, Error> processNode(const aiNode *loadedNode) { // NOLINT(*-no-recursion)
        Node resultNode;
        resultNode.transform = UNPACK_MAT4(loadedNode->mTransformation);

        resultNode.children.reserve(loadedNode->mNumChildren);
        for (unsigned int i = 0; i < loadedNode->mNumChildren; i++) {
            std::expected<Node, Error> result = processNode(loadedNode->mChildren[i]);
            if (!result.has_value())
                return std::unexpected(FW_ERROR(result.error(), "Failed to load child node "+std::to_string(i)));
            resultNode.children.push_back(result.value());
        }

        return resultNode;
    }

    std::expected<Mesh, Error> processMesh(const aiMesh *loadedMesh) {
        std::vector<MeshVertex> vertices;
        std::vector<unsigned int> indices;
        const unsigned int materialIndex = loadedMesh->mMaterialIndex;

        for (unsigned int i = 0; i < loadedMesh->mNumVertices; i++) {
            MeshVertex vertex{};
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

    std::expected<Material, Error> processMaterial(const aiMaterial *loadedMaterial) {
        Material resultMaterial;

        aiString path;
        if (loadedMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            resultMaterial.diffusePath = path.C_Str();
        // else
        //     logWarn("Material %s does not have a diffuse texture", loadedMaterial->GetName().C_Str());
        if (loadedMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
            resultMaterial.specularPath = path.C_Str();
        // else
        //     logWarn("Material %s does not have a specular texture", loadedMaterial->GetName().C_Str());

        // TODO: This does not make sense in the context of our renderer
        if (loadedMaterial->Get(AI_MATKEY_REFLECTIVITY, resultMaterial.shininess) != AI_SUCCESS) {
            // logWarn("Material %s does not have shininess", loadedMaterial->GetName().C_Str());
            resultMaterial.shininess = 32.0f;
        }

        return resultMaterial;
    }
#pragma endregion


#pragma region Scene Rendering
    void Mesh::bindGlMesh() const {
        glBindVertexArray(VAO);
    }

    std::expected<void, Error> Scene::Draw(ResourceManager &resourceManager, const ShaderProgram &shader, const glm::mat4 &modelTransform) const {
        // TODO: Only do unique per-scene stuff here, and don't double-use the shader
        shader.use();
        for (const Mesh &mesh: meshes) {
            auto matRet = materials[mesh.materialIndex].PopulateShader(shader, resourceManager);
            if (!matRet.has_value())
                return std::unexpected(FW_ERROR(matRet.error(), "Failed to populate shader with material"));

            const auto model = rootNode.transform * modelTransform;
            shader.setMat4("model", model);
            shader.setMat3("mTransposed", glm::mat3(glm::transpose(glm::inverse(model))));

            mesh.bindGlMesh();
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        return {};
    }

    std::expected<void, Error> Material::PopulateShader(const ShaderProgram &shader, ResourceManager &resourceManager) const {
        shader.setFloat("material.shininess", shininess);

        const auto diffuse = resourceManager.loadTexture(diffusePath);
        shader.setInt("material.texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse->textureID);

        const auto specular = resourceManager.loadTexture(specularPath);
        shader.setInt("material.texture_specular", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular->textureID);

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
        std::vector<MeshVertex> &&vertices,
        std::vector<unsigned int> &&indices,
        const unsigned int materialIndex
    ) : vertices(std::move(vertices)), indices(std::move(indices)), materialIndex(materialIndex) {
        setupGlMesh();
    }

    void Mesh::setupGlMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set all the properties of the vertices
#define ENABLE_F_VERTEX_ATTRIB(index, member) \
        glEnableVertexAttribArray(index); \
        glVertexAttribPointer(index, \
            sizeof(MeshVertex::member) / sizeof(float), GL_FLOAT, \
            GL_FALSE, \
            sizeof(MeshVertex), \
            reinterpret_cast<void *>(offsetof(MeshVertex, member)))

        ENABLE_F_VERTEX_ATTRIB(0, Position);
        ENABLE_F_VERTEX_ATTRIB(1, Normal);
        ENABLE_F_VERTEX_ATTRIB(2, TexCoords);
        ENABLE_F_VERTEX_ATTRIB(3, Color);
#undef ENABLE_F_VERTEX_ATTRIB
    }


    Mesh::~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    Mesh::Mesh(Mesh &&other) noexcept {
        BUFFERS_MV_FROM_TO(other, this);

        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        materialIndex = other.materialIndex;
    }
    Mesh &Mesh::operator=(Mesh &&other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);

            BUFFERS_MV_FROM_TO(other, this);

            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            materialIndex = other.materialIndex;
        }
        return *this;
    }

#pragma endregion
};

