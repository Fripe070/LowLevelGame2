#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <expected>
#include <string>
#include <vector>
#include <glm/fwd.hpp>

namespace Engine {
    class ShaderProgram {
    protected:  // ShaderProgram should never be instantiated directly, only through derived classes
        explicit ShaderProgram(const std::vector<std::pair<std::string, unsigned int>> &filePaths);
    public:
        unsigned int programID;

        ~ShaderProgram();

        // Non-copyable
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        // Moveable
        ShaderProgram(ShaderProgram&& other) noexcept;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept;

        void use() const;
        [[nodiscard]] int getUniformLoc(const std::string &name) const;

    private:
        static std::expected<unsigned int, std::string> loadShader(
            const std::string &filePath,
            unsigned int shaderType
        );
        static std::expected<std::string, std::string> preprocessSource(std::string shaderSrc);

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

        std::expected<void, std::string> bindUniformBlock(const std::string &name, unsigned int bindingPoint) const;
    };
}

#endif
