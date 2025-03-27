#pragma once
#include <expected>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/fwd.hpp>

#include "engine/util/error.h"


namespace Engine
{
    enum ShaderType
    {
        FRAGMENT = GL_FRAGMENT_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        VERTEX = GL_VERTEX_SHADER,
        COMPUTE = GL_COMPUTE_SHADER,
        TESS_CONTROL = GL_TESS_CONTROL_SHADER,
        TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
    };

    [[nodiscard]] std::expected<unsigned int, Error> loadShaderFile(const std::string& filePath, unsigned int shaderType);
    [[nodiscard]] std::expected<unsigned int, Error> loadShaderString(const std::string& shaderSrc, unsigned int shaderType);
    [[nodiscard]] std::expected<std::string, Error> preprocessShaderSource(std::string shaderSrc);
    [[nodiscard]] std::expected<unsigned int, Error> compileShader(const char* shaderSource, unsigned int shaderType);

    class ShaderProgram {
    private:
        unsigned int programID;
    public:
        explicit ShaderProgram(const std::vector<unsigned int> &shaders);
        ~ShaderProgram();

        // Non-copyable
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        // Moveable
        ShaderProgram(ShaderProgram&& other) noexcept;
        ShaderProgram& operator=(ShaderProgram&& other) noexcept;

        void use() const;
        [[nodiscard]] int getUniformLoc(const std::string &name) const;


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

        [[nodiscard]] std::expected<void, Error> bindUniformBlock(const std::string &name, unsigned int bindingPoint) const;
    };
}

