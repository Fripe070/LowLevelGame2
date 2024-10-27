#ifndef SHADER_H
#define SHADER_H
#include <expected>
#include <string>
#include <vector>
#include <glm/fwd.hpp>

namespace Engine {
    class ShaderProgram {
    public:
        unsigned int programID;

        ShaderProgram(const std::string &vertexFilePath, const std::string &fragmentFilePath);
        ShaderProgram(const std::string &vertexFilePath, const std::string &geometryFilePath, const std::string &fragmentFilePath);
        ShaderProgram(const std::vector<std::pair<std::string, unsigned int>> &filePaths);
        ~ShaderProgram();

        void use() const;

        unsigned int getUniformLoc(const std::string &name) const;

    private:
        void Constructor(const std::vector<std::pair<std::string, unsigned int>> &filePaths);

        static std::expected<unsigned int, std::string> loadShader(
            const std::string &filePath,
            const unsigned int shaderType
        );
        static std::expected<unsigned int, std::string> programFromMultiple(
            const std::vector<std::pair<std::string, unsigned int>> &shaders
        );

    public:
        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;

        void setVec2(const std::string &name, const glm::vec2 &value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec4(const std::string &name, const glm::vec4 &value) const;
        void setVec2(const std::string &name, float x, float y) const;
        void setVec3(const std::string &name, float x, float y, float z) const;
        void setVec4(const std::string &name, float x, float y, float z, float w) const;

        void setMat2(const std::string &name, const glm::mat2 &mat) const;
        void setMat3(const std::string &name, const glm::mat3 &mat) const;
        void setMat4(const std::string &name, const glm::mat4 &mat) const;
    };
}

#endif //SHADER_H