#include "shader.h"

#include <vector>
#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "logging.h"
#include "loader/generic.h"

namespace Engine {
    Shader::Shader(const std::string &vertexFilePath, const std::string &fragmentFilePath) {
        const auto vertShaderSrc = Loader::readTextFile(vertexFilePath);
        if (!vertShaderSrc) {
            logError("Failed to read vertex shader file: %s", vertexFilePath.c_str());
            return;
        }
        const auto fragShaderSrc = Loader::readTextFile(fragmentFilePath);
        if (!fragShaderSrc) {
            logError("Failed to read fragment shader file: %s", fragmentFilePath.c_str());
            return;
        }

        const unsigned int vertShaderID = glCreateShader(GL_VERTEX_SHADER);
        const unsigned int fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        const char *vertexSource = vertShaderSrc.value().c_str();
        glShaderSource(vertShaderID, 1, &vertexSource, nullptr);
        glCompileShader(vertShaderID);
        if (logShaderError(vertShaderID))
            return;

        const char *fragmentSource = fragShaderSrc.value().c_str();
        glShaderSource(fragShaderID, 1, &fragmentSource, nullptr);
        glCompileShader(fragShaderID);
        if (logShaderError(fragShaderID))
            return;

        programID = glCreateProgram();
        glAttachShader(programID, vertShaderID);
        glAttachShader(programID, fragShaderID);
        glLinkProgram(programID);
        if (logProgramError(programID))
            return;

        glDeleteShader(vertShaderID);
        glDeleteShader(fragShaderID);
    }

    Shader::~Shader() {
        glDeleteProgram(programID);
    }

    void Shader::use() const {
        glUseProgram(programID);
    }

    unsigned int Shader::getUniformLoc(const std::string &name) const {
        return glGetUniformLocation(programID, name.c_str());
    }

    void Shader::setBool(const std::string &name, const bool value) const {
        glUniform1i(getUniformLoc(name), static_cast<int>(value));
    }
    void Shader::setInt(const std::string &name, const int value) const {
        glUniform1i(getUniformLoc(name), value);
    }
    void Shader::setFloat(const std::string &name, const float value) const {
        glUniform1f(getUniformLoc(name), value);
    }

    void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(getUniformLoc(name), 1, &value[0]);
    }
    void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(getUniformLoc(name), 1, &value[0]);
    }
    void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(getUniformLoc(name), 1, &value[0]);
    }

    void Shader::setVec2(const std::string &name, float x, float y) const {
        glUniform2f(getUniformLoc(name), x, y);
    }
    void Shader::setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(getUniformLoc(name), x, y, z);
    }
    void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(getUniformLoc(name), x, y, z, w);
    }

    void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }
    void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }
    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }

    bool Shader::logShaderError(const unsigned int &shaderID) {
        GLint result = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
        if (result == GL_TRUE)
            return false;

        GLint infoLogLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
        logError("Shader compilation failed: %s", infoLog.data());
        return true;
    }

    bool Shader::logProgramError(const unsigned int &programID) {
        GLint result = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &result);
        if (result == GL_TRUE)
            return false;

        GLint infoLogLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> infoLog(infoLogLength);
        glGetProgramInfoLog(programID, infoLogLength, nullptr, infoLog.data());
        logError("Program linking failed: %s", infoLog.data());
        return true;
    }
}
