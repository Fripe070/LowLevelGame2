#pragma once
#include <expected>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/fwd.hpp>

#include "engine/util/error.h"

namespace Resource
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

    class Shader {
    private:
        unsigned int programID;
    public:
        /*!
         * Creates a shader program from a set of shader stages.
         * @param shaders The shader stages to link into a program.
         * @warning Passed stages will be deleted when we go out of scope. Do not use them elsewhere.
         */
        explicit Shader(const std::vector<unsigned int> &shaders);
        /*!
         * Creates a shader object around an existing shader program ID.
         * @param shaderProgramID The ID of the shader program to use.
         * @note There is little to no reason to use this constructor outside the engine itself.
         */
        explicit Shader(unsigned int shaderProgramID);
        ~Shader();

        // Non-copyable
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        // Moveable
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        void use() const;
        // TODO: Can error, but would mean all the setter functions also have to return expected
        [[nodiscard]] int getUniformLocation(const std::string &name) const;
        [[nodiscard]] Expected<void> bindUniformBlock(const std::string &name, unsigned int bindingPoint) const;

        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;

        void setVec2(const std::string &name, const glm::vec2 &value) const;
        void setVec2(const std::string &name, float x, float y) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec3(const std::string &name, float x, float y, float z) const;
        void setVec4(const std::string &name, const glm::vec4 &value) const;
        void setVec4(const std::string &name, float x, float y, float z, float w) const;

        void setMat2(const std::string &name, const glm::mat2 &mat) const;
        void setMat3(const std::string &name, const glm::mat3 &mat) const;
        void setMat4(const std::string &name, const glm::mat4 &mat) const;
    };
}

namespace Resource::Loading
{
    /*!
     * Loads a GLSL shader from a file and compiles it.
     * @param filePath The path to the GLSL shader file.
     * @param shaderType The type of shader to load (for example vertex or fragment).
     * @return The shader ID if successful, or an error.
     * @note Sources will be preprocessed.
     * @note A shader here is not a complete shader *program*, but a single shader stage.
     *       Pass the resulting shader ID(s) to the Shader constructor to link them into a program.
     */
    [[nodiscard]] std::expected<unsigned int, Error> loadGLShaderFile(const std::string& filePath, ShaderType shaderType);
    /*!
     * Loads a GLSL shader from a string and compiles it.
     * @param shaderSrc The GLSL shader source code.
     * @param shaderType The type of shader to load (for example vertex or fragment).
     * @return The shader ID if successful, or an error.
     * @note Sources will be preprocessed.
     * @note A shader here is not a complete shader *program*, but a single shader stage.
     *       Pass the resulting shader ID(s) to the Shader constructor to link them into a program.
     */
    [[nodiscard]] std::expected<unsigned int, Error> loadGLShaderSource(const std::string& shaderSrc, ShaderType shaderType);
}
