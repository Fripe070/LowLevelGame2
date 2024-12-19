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
            logError("Failed to create shader program" NL_INDENT "%s", programID.error().c_str());
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
        std::expected<std::string, std::string> shaderSrc = Loader::readTextFile(filePath);
        if (!shaderSrc.has_value())
            return std::unexpected(FW_UNEXP(shaderSrc, "Failed to read shader file"));
        shaderSrc = preprocessSource(shaderSrc.value());
        // logWarn("Shader source: %s", shaderSrc.value().c_str());
        if (!shaderSrc.has_value())
            return std::unexpected(FW_UNEXP(shaderSrc, std::string("Failed to preprocess shader source")));

        const unsigned int shaderID = glCreateShader(shaderType);
        const char *shaderSource = shaderSrc.value().c_str();
        glShaderSource(shaderID, 1, &shaderSource, nullptr);
        glCompileShader(shaderID);

        int result = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
        if (result == GL_TRUE)
            return shaderID;

        int infoLogLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength == 0) {
            glDeleteShader(shaderID);  // Prevent leak if we failed to compile
            return UNEXPECTED_REF("Shader compilation failed: No info log available");
        }

        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog.data());
        glDeleteShader(shaderID);  // Prevent leak if we failed to compile
        return UNEXPECTED_REF("Shader compilation failed: " + std::string(infoLog.data()));
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
                return std::unexpected(FW_UNEXP(shaderID, "Failed to compile shader"));
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
            return UNEXPECTED_REF("Program linking failed: No info log available");
        }

        std::vector<char> infoLog(infoLogLength);
        glGetProgramInfoLog(progID, infoLogLength, nullptr, infoLog.data());
        glDeleteProgram(progID);  // Automatically detaches shaders, we don't need to loop through them
        return UNEXPECTED_REF("Program linking failed: " + std::string(infoLog.data()));
    }

    std::expected<std::string, std::string> ShaderProgram::preprocessSource(std::string shaderSrc) {
        constexpr auto includeDirective = "#include";
        constexpr auto includeDirectiveLength = strlen(includeDirective);

        std::vector<std::string> processedFiles; // Avoid infinite recursion and unnecessary file reads

        std::string::size_type cursor = 0;
        std::string::size_type findStart = cursor;
        while ((findStart = shaderSrc.find(includeDirective, findStart)) != std::string::npos) {
            cursor = findStart;
            // Exit if we are in the middle of a line (ignoring leading whitespace)
            while (shaderSrc[cursor - 1] == ' ' || shaderSrc[cursor - 1] == '\t')
                cursor--;
            if (shaderSrc[findStart - 1] != '\n') {
                logWarn("Invalid include directive at " + std::to_string(findStart) + ". Expected start of line:" + shaderSrc.substr(findStart-10, 40));
                findStart++; // So we won't find the same include directive again
                continue;
            }
            cursor = findStart;

#define IN_BOUNDS cursor < shaderSrc.size()

            cursor += includeDirectiveLength;
            if (shaderSrc[cursor] != ' ' && shaderSrc[cursor] != '\t') {
                logWarn("Invalid include directive. Expected whitespace after directive");
                continue;
            }
            cursor++;
            while (IN_BOUNDS && shaderSrc[cursor] == ' ' || shaderSrc[cursor] == '\t')
                cursor++;

            if (shaderSrc[cursor] != '"') {
                logWarn("Invalid include directive. Expected opening quote");
                continue;
            }
            cursor++;
            const auto pathStart = cursor;
            while (IN_BOUNDS && shaderSrc[cursor] != '"' && shaderSrc[cursor] != '\n')
                cursor++;
            if (shaderSrc[cursor] != '"') {
                logWarn("Invalid include directive. Expected closing quote");
                continue;
            }
            const auto includePath = shaderSrc.substr(pathStart, cursor - pathStart);
            cursor++;

            if (std::ranges::find(processedFiles, includePath) != processedFiles.end()) {
                shaderSrc.replace(findStart, cursor - findStart, "// ignoring #include " + includePath + " (already included)");
                continue;
            }
            logDebug("Processing include file \"%s\"", includePath.c_str());
            processedFiles.push_back(includePath);
            std::expected<std::string, std::string> includeSrc = Loader::readTextFile(includePath);
            if (!includeSrc.has_value())
                return std::unexpected(FW_UNEXP(includeSrc,
                    "Failed to read include: " + shaderSrc.substr(findStart, cursor - findStart)));
            // Replace the include directive with the actual source
            shaderSrc.replace(findStart, cursor - findStart, includeSrc.value());
        }
#undef IN_BOUNDS

        return shaderSrc;
    }

    void ShaderProgram::use() const {
        glUseProgram(programID);
    }

    int ShaderProgram::getUniformLoc(const std::string &name) const {
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

    std::expected<void, std::string> ShaderProgram::bindUniformBlock(const std::string &name, const unsigned int bindingPoint) const {
        const unsigned int blockIndex = glGetUniformBlockIndex(programID, name.c_str());
        if (blockIndex == GL_INVALID_INDEX)
            return UNEXPECTED_REF("Failed to get uniform block index");

        glUniformBlockBinding(programID, blockIndex, bindingPoint);
        return {};
    }

}
