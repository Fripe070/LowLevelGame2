#include "shader.h"

#include <glm/mat3x3.hpp>

#include "engine/util/file.h"

namespace Resource {
    Shader::Shader(const unsigned int shaderProgramID) {
        programID = shaderProgramID;
    }

    Shader::Shader(const std::vector<unsigned int>& shaders) {
        const unsigned int progID = glCreateProgram();
        // Attach all shaders
        for (const auto &shaderID : shaders) {
            glAttachShader(progID, shaderID);
            glDeleteShader(shaderID);  // Flagged for deletion when no longer attached to anything
        }

        glLinkProgram(progID);

        int result = GL_FALSE;
        glGetProgramiv(progID, GL_LINK_STATUS, &result);
        if (result == GL_TRUE) {
            for (const auto &shaderID : shaders)
                // Detach shaders, we only need what is linked in the program now
                // Shaders flagged for deletion will be deleted when no longer attached to anything
                glDetachShader(progID, shaderID);
            programID = progID;
            return;
        }

        // Linking failed
        int infoLogLength = 0;
        glGetProgramiv(progID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength == 0) {
            glDeleteProgram(progID);  // Automatically detaches shaders, we don't need to loop through them
            throw std::runtime_error("Program linking failed. No info log available");
        }

        std::vector<char> infoLog(infoLogLength);
        glGetProgramInfoLog(progID, infoLogLength, nullptr, infoLog.data());
        glDeleteProgram(progID);  // Automatically detaches shaders, we don't need to loop through them
        throw std::runtime_error("Program linking failed with: " + std::string(infoLog.data()));
    }

    Shader::~Shader() {
        glDeleteProgram(programID);
    }

    Shader::Shader(Shader &&other) noexcept {
        programID = other.programID;
        other.programID = 0;
    }
    Shader &Shader::operator=(Shader &&other) noexcept {
        if (this != &other) {
            glDeleteProgram(programID);
            programID = other.programID;
            other.programID = 0;
        }
        return *this;
    }

    void Shader::use() const {
        glUseProgram(programID);
    }

    int Shader::getUniformLocation(const std::string& name) const {
        return glGetUniformLocation(programID, name.c_str());
    }

    Expected<void> Shader::bindUniformBlock(const std::string& name, const unsigned int bindingPoint) const {
        const unsigned int blockIndex = glGetUniformBlockIndex(programID, name.c_str());
        if (blockIndex == GL_INVALID_INDEX)
            return std::unexpected(ERROR("Failed to get uniform block index for " + name));
        glUniformBlockBinding(programID, blockIndex, bindingPoint);
        return {};
    }

#pragma region Setters
    void Shader::setBool(const std::string &name, const bool value) const {
        glUniform1i(getUniformLocation(name), static_cast<int>(value));
    }
    void Shader::setInt(const std::string &name, const int value) const {
        glUniform1i(getUniformLocation(name), value);
    }
    void Shader::setFloat(const std::string &name, const float value) const {
        glUniform1f(getUniformLocation(name), value);
    }

    void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(getUniformLocation(name), 1, &value[0]);
    }
    void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(getUniformLocation(name), 1, &value[0]);
    }
    void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(getUniformLocation(name), 1, &value[0]);
    }

    void Shader::setVec2(const std::string &name, const float x, const float y) const {
        glUniform2f(getUniformLocation(name), x, y);
    }
    void Shader::setVec3(const std::string &name, const float x, const float y, const float z) const {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    void Shader::setVec4(const std::string &name, const float x, const float y, const float z, const float w) const {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }

    void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
#pragma endregion
}

namespace Resource::Loading
{
    [[nodiscard]] std::expected<std::string, Error> preprocessShaderSource(std::string shaderSrc);
    [[nodiscard]] std::expected<unsigned int, Error> compileSingleShader(const char* shaderSource, unsigned int shaderType);

    std::expected<unsigned int, Error> loadGLShaderFile(
        const std::string& filePath,
        const ShaderType shaderType
    ) {
        std::expected<std::string, Error> shaderSrc = readTextFile(filePath);
        if (!shaderSrc.has_value())
            return std::unexpected(FW_ERROR(shaderSrc.error(), "Failed to read shader file"));
        shaderSrc = preprocessShaderSource(shaderSrc.value());
        if (!shaderSrc.has_value())
            return std::unexpected(FW_ERROR(shaderSrc.error(), std::string("Failed to preprocess shader source")));
        return compileSingleShader(shaderSrc.value().c_str(), shaderType);
    }

    std::expected<unsigned int, Error> loadGLShaderSource(const std::string& shaderSrc, const ShaderType shaderType) {
        std::expected<std::string, Error> shaderSource = preprocessShaderSource(shaderSrc);
        if (!shaderSource.has_value())
            return std::unexpected(FW_ERROR(shaderSource.error(), std::string("Failed to preprocess shader source")));
        return compileSingleShader(shaderSource.value().c_str(), shaderType);
    }

    std::expected<unsigned int, Error> compileSingleShader(
        const char* shaderSource,
        const unsigned int shaderType
    ) {
        const unsigned int shaderID = glCreateShader(shaderType);
        glShaderSource(shaderID, 1, &shaderSource, nullptr);
        glCompileShader(shaderID);

        int result = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
        if (result == GL_TRUE)
            return shaderID;

        // Compilation failed
        int infoLogLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength == 0) {
            glDeleteShader(shaderID);  // Prevent leak if we failed to compile
            return std::unexpected(ERROR("Shader compilation failed: No info log available"));
        }
        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
        glDeleteShader(shaderID);  // Prevent leak if we failed to compile
        return std::unexpected(ERROR("Shader compilation failed: " + std::string(infoLog.data())));
    }

    std::expected<std::string, Error> preprocessShaderSource(std::string shaderSrc) {
        constexpr auto includeDirective = "#include";
        const auto includeDirectiveLength = strlen(includeDirective);

        std::vector<std::string> processedFiles; // Avoid infinite recursion and unnecessary file reads

        std::string::size_type cursor = 0;
        std::string::size_type findStart = cursor;
        while ((findStart = shaderSrc.find(includeDirective, findStart)) != std::string::npos) {
            cursor = findStart;
            // Exit if we are in the middle of a line (ignoring leading whitespace)
            while (shaderSrc[cursor - 1] == ' ' || shaderSrc[cursor - 1] == '\t')
                cursor--;
            if (shaderSrc[findStart - 1] != '\n') {
                spdlog::warn(
                    "Invalid include directive at " + std::to_string(findStart) + ". "
                         "Expected start of line:" + shaderSrc.substr(findStart-10, 40));
                findStart++; // So we won't find the same include directive again
                continue;
            }
            cursor = findStart;

#define IN_BOUNDS cursor < shaderSrc.size()

            cursor += includeDirectiveLength;
            if (shaderSrc[cursor] != ' ' && shaderSrc[cursor] != '\t') {
                spdlog::warn("Invalid include directive. Expected whitespace after directive");
                continue;
            }
            cursor++;
            while (IN_BOUNDS && (shaderSrc[cursor] == ' ' || shaderSrc[cursor] == '\t'))
                cursor++;

            if (shaderSrc[cursor] != '"') {
                spdlog::warn("Invalid include directive. Expected opening quote");
                continue;
            }
            cursor++;
            const auto pathStart = cursor;
            while (IN_BOUNDS && shaderSrc[cursor] != '"' && shaderSrc[cursor] != '\n')
                cursor++;
            if (shaderSrc[cursor] != '"') {
                spdlog::warn("Invalid include directive. Expected closing quote");
                continue;
            }
            const auto includePath = shaderSrc.substr(pathStart, cursor - pathStart);
            cursor++;

            if (std::ranges::find(processedFiles, includePath) != processedFiles.end()) {
                shaderSrc.replace(findStart, cursor - findStart, "// ignoring #include " + includePath + " (already included)");
                continue;
            }
            spdlog::debug("Processing include file \"%s\"", includePath.c_str());
            processedFiles.push_back(includePath);
            auto includeSrc = readTextFile(includePath);
            if (!includeSrc.has_value())
                return std::unexpected(FW_ERROR(
                    includeSrc.error(),
                    "Failed to read include: " + shaderSrc.substr(findStart, cursor - findStart)));
            // Replace the include directive with the actual source
            shaderSrc.replace(findStart, cursor - findStart, includeSrc.value());
        }
#undef IN_BOUNDS

        return shaderSrc;
    }
}
