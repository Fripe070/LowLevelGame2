#include "scene.h"

#include <assimp/cimport.h>
#include <engine/logging.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

        Scene scene;

        // Load the node tree
        std::expected<Node, std::string> result = processNode(loadedNode->mRootNode);
        if (!result.has_value())
            return std::unexpected(result.error());
        scene.rootNode = result.value();

        // Load all the meshes
        for (unsigned int i = 0; i < loadedNode->mNumMeshes; i++) {
            std::expected<Mesh, std::string> mesh = processMesh(loadedNode->mMeshes[i]);
            if (!mesh.has_value())
                return std::unexpected(mesh.error());
            scene.meshes.push_back(mesh.value());
        }

        // Load all the materials
        for (unsigned int i = 0; i < loadedNode->mNumMaterials; i++) {
            std::expected<Material, std::string> material = processMaterial(loadedNode->mMaterials[i]);
            if (!material.has_value())
                return std::unexpected(material.error());
            scene.materials.push_back(material.value());
        }

#ifndef NDEBUG
        logDebug("Loaded scene \"%s\" in %d ms", path.c_str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());
#else
        logDebug("Loaded scene \"%s\"", path.c_str());
#endif
        return scene;
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
        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        const unsigned int materialIndex = loadedMesh->mMaterialIndex;

        for (unsigned int i = 0; i < loadedMesh->mNumVertices; i++) {
            Mesh::Vertex vertex;
            vertex.Position = UNPACK_VEC3(loadedMesh->mVertices[i]);

            if (loadedMesh->HasNormals())
                vertex.Normal = UNPACK_VEC3(loadedMesh->mNormals[i]);

            // 0 since we only support a single set of texture coordinates atm (the first one)
            if (loadedMesh->HasTextureCoords(0))
                vertex.TexCoords = UNPACK_VEC3(loadedMesh->mTextureCoords[0][i]);

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

        return Mesh(vertices, indices, materialIndex);
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

        if (loadedMaterial->Get(AI_MATKEY_REFLECTIVITY, resultMaterial.shininess) != AI_SUCCESS) {
            logWarn("Material %s does not have shininess", loadedMaterial->GetName().C_Str());
            resultMaterial.shininess = 32.0f;
        }

        return resultMaterial;
    }



    Mesh::Mesh(
        const std::vector<Vertex> &vertices,
        const std::vector<unsigned int> &indices,
        const unsigned int materialIndex
    ) {
        this->vertices = vertices;
        this->indices = indices;
        this->materialIndex = materialIndex;

        setupGlMesh();
    }
    Mesh::~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
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

    void Mesh::LoadGlMesh() const {
        glBindVertexArray(VAO);
    }

    std::expected<void, std::string> Scene::Draw(Manager::TextureManager &textureManager, const ShaderProgram &shader) const {
        shader.use();
        for (const Mesh &mesh: meshes) {
            auto mRet = materials[mesh.materialIndex].PopulateShader(shader, textureManager);
            if (!mRet.has_value())
                return std::unexpected(mRet.error());

            mesh.LoadGlMesh();
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        }
        return {};
    }

    std::expected<void, std::string> Material::PopulateShader(const ShaderProgram &shader, Manager::TextureManager &textureManager) const {
        shader.setFloat("material.shininess", shininess);

        std::expected<unsigned int, std::string> diffuse = textureManager.getTexture(diffusePath);
        if (!diffuse.has_value())
            return std::unexpected(diffuse.error());
        shader.setInt("material.texture_diffuse", diffuse.value_or(textureManager.errorTexture));

        std::expected<unsigned int, std::string> specular = textureManager.getTexture(specularPath);
        if (!specular.has_value())
            return std::unexpected(specular.error());
        shader.setInt("material.texture_specular", specular.value_or(textureManager.errorTexture));

        return {};
    }
};
