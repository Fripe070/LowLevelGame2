#include "shader.h"

#include <stdexcept>
#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "generic.h"
#include <engine/logging.h>
#ifndef NDEBUG
#include <chrono>
#endif

namespace Engine {
    ShaderProgram::ShaderProgram(
        const std::string &vertexFilePath,
        const std::string &fragmentFilePath
    ) {
        Constructor({
            {vertexFilePath, GL_VERTEX_SHADER},
            {fragmentFilePath, GL_FRAGMENT_SHADER}
        });
    }
    ShaderProgram::ShaderProgram(
        const std::string &vertexFilePath,
        const std::string &geometryFilePath,
        const std::string &fragmentFilePath
    ) {
        Constructor({
            {vertexFilePath, GL_VERTEX_SHADER},
            {geometryFilePath, GL_GEOMETRY_SHADER},
            {fragmentFilePath, GL_FRAGMENT_SHADER}
        });
    }
    ShaderProgram::ShaderProgram(const std::vector<std::pair<std::string, unsigned int>> &filePaths) {
        Constructor(filePaths);
    }

    void ShaderProgram::Constructor(const std::vector<std::pair<std::string, unsigned int>> &filePaths) {
        const std::expected<unsigned int, std::string> programID = programFromMultiple(filePaths);
        if (!programID.has_value()) {
            logError("Failed to create shader program: %s", programID.error().c_str());
            throw std::runtime_error(programID.error());
        }
        this->programID = programID.value();
    }
    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(programID);
    }

    ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept {
        programID = other.programID;
        other.programID = 0;
    }
    ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept {
        if (this != &other) {
            glDeleteProgram(programID);
            programID = other.programID;
            other.programID = 0;
        }
        return *this;
    }

    std::expected<unsigned int, std::string> ShaderProgram::loadShader(
        const std::string &filePath,
        const unsigned int shaderType
    ) {
        const std::expected<std::string, std::string> shaderSrc = Loader::readTextFile(filePath);
        if (!shaderSrc.has_value())
            return std::unexpected("Failed to read shader file: " + shaderSrc.error());

        const unsigned int shaderID = glCreateShader(shaderType);
        const char *shaderSource = shaderSrc.value().c_str();
        glShaderSource(shaderID, 1, &shaderSource, nullptr);
        glCompileShader(shaderID);

        int result = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
        if (result == GL_TRUE)
            return shaderID;
        glDeleteShader(shaderID);  // Prevent leak if we failed to compile

        int infoLogLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength == 0)
            return std::unexpected("Shader compilation failed: No info log available");

        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
        return std::unexpected("Shader compilation failed: " + std::string(infoLog.data()));
    }

    std::expected<unsigned int, std::string> ShaderProgram::programFromMultiple(
        const std::vector<std::pair<std::string, unsigned int>> &shaders
        ) {

        std::vector<unsigned int> shaderIDs;
        shaderIDs.reserve(shaders.size());

#ifndef NDEBUG
        const auto startTimer = std::chrono::high_resolution_clock::now();
#endif
        // Load all shaders
        for (const auto &[filePath, shaderType] : shaders) {
#ifndef NDEBUG
            const auto shaderStartTimer = std::chrono::high_resolution_clock::now();
#endif
            const std::expected<unsigned int, std::string> shaderID = loadShader(filePath, shaderType);
#ifndef NDEBUG
            logDebug("Compiled shader \"%s\" in %.2fms",
                filePath.c_str(),
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - shaderStartTimer).count() / 1000.0);
#endif
            if (!shaderID.has_value()) {
                for (const auto &id : shaderIDs)
                    glDeleteShader(id);
                return std::unexpected(shaderID.error());
            }
            shaderIDs.push_back(shaderID.value());
        }

        const unsigned int progID = glCreateProgram();
        // Attach all shaders
        for (const auto &shaderID : shaderIDs) {
            glAttachShader(progID, shaderID);
            glDeleteShader(shaderID);  // Flagged for deletion when no longer attached to anything
        }

        glLinkProgram(progID);

        int result = GL_FALSE;
        glGetProgramiv(progID, GL_LINK_STATUS, &result);
        if (result == GL_TRUE) {
            for (const auto &shaderID : shaderIDs)
                glDetachShader(progID, shaderID);  // Detach and delete shaders, we only need what is linked in the program now
#ifndef NDEBUG
            logDebug("Linked shader program in %dms with %d shaders",
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTimer).count(),
                shaders.size());
#endif
            return progID;
        }

        int infoLogLength = 0;
        glGetProgramiv(progID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength == 0) {
            glDeleteProgram(progID);  // Automatically detaches shaders, we don't need to loop through them
            return std::unexpected("Program linking failed: No info log available");
        }

        std::vector<char> infoLog(infoLogLength);
        glGetProgramInfoLog(progID, infoLogLength, nullptr, infoLog.data());
        glDeleteProgram(progID);  // Automatically detaches shaders, we don't need to loop through them
        return std::unexpected("Program linking failed: " + std::string(infoLog.data()));
    }

    void ShaderProgram::use() const {
        glUseProgram(programID);
    }

    unsigned int ShaderProgram::getUniformLoc(const std::string &name) const {
        return glGetUniformLocation(programID, name.c_str());
    }

    void ShaderProgram::setBool(const std::string &name, const bool value) const {
        glUniform1i(getUniformLoc(name), static_cast<int>(value));
    }
    void ShaderProgram::setInt(const std::string &name, const int value) const {
        glUniform1i(getUniformLoc(name), value);
    }
    void ShaderProgram::setFloat(const std::string &name, const float value) const {
        glUniform1f(getUniformLoc(name), value);
    }

    void ShaderProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(getUniformLoc(name), 1, &value[0]);
    }
    void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(getUniformLoc(name), 1, &value[0]);
    }
    void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(getUniformLoc(name), 1, &value[0]);
    }

    void ShaderProgram::setVec2(const std::string &name, const float x, const float y) const {
        glUniform2f(getUniformLoc(name), x, y);
    }
    void ShaderProgram::setVec3(const std::string &name, const float x, const float y, const float z) const {
        glUniform3f(getUniformLoc(name), x, y, z);
    }
    void ShaderProgram::setVec4(const std::string &name, const float x, const float y, const float z, const float w) const {
        glUniform4f(getUniformLoc(name), x, y, z, w);
    }

    void ShaderProgram::setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }
    void ShaderProgram::setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }
    void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(getUniformLoc(name), 1, GL_FALSE, &mat[0][0]);
    }
}
