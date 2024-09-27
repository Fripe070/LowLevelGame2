#include "scene_loader.h"

#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "logging.h"
#include "shader.h"
#include "utility.h"


SceneLoader::Mesh::Mesh(
    const std::vector<Vertex> &vertices,
    const std::vector<unsigned int> &indices,
    const std::vector<Texture> &textures
) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupGlMesh();
}

void SceneLoader::Mesh::setupGlMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Set all the properties of the vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), nullptr); // first so offset is 0

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, Normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, TexCoords)));

    glBindVertexArray(0);
}

#define DIFFUSE_TEX_NAME "texture_diffuse"
#define SPECULAR_TEX_NAME "texture_specular"

void SceneLoader::Mesh::Draw(const Shader &shader) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // Textures are offset by 1 each, if we overflow... well...
        std::string number;
        std::string name = textures[i].type;
        if (name == DIFFUSE_TEX_NAME)
            number = std::to_string(diffuseNr++);
        else if (name == SPECULAR_TEX_NAME)
            number = std::to_string(specularNr++);

        shader.setInt(("material." + name + number).c_str(), i); // material.texture_diffuse1
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(GL_ZERO);
}

SceneLoader::Model::Model(const char *path) {
    loadModel(path);
}

void SceneLoader::Model::loadModel(const std::string &path) {
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

std::expected<void, std::string> SceneLoader::Model::processNode(const aiNode *node, const aiScene *scene) {
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

std::expected<SceneLoader::Mesh, std::string> SceneLoader::Model::processMesh(const aiMesh *mesh, const aiScene *scene) {
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

std::vector<SceneLoader::Texture> SceneLoader::Model::loadMaterialTextures(
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


void SceneLoader::Model::Draw(const Shader &shader) const {
    for (auto mesh: meshes)
        mesh.Draw(shader);
}
