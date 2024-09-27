#ifndef MESH_H
#define MESH_H
#include <expected>
#include <string>
#include <vector>
#include <assimp/material.h>
#include <gl/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;
class Shader;

namespace SceneLoader {
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Texture {
        GLuint id;
        std::string type;
        std::string path;
    };

    class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<Texture> &textures);

        void Draw(const Shader &shader) const;

    private:
        GLuint VAO, VBO, EBO;

        void setupGlMesh();
    };

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


#endif //MESH_H
