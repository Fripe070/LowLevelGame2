#ifndef SHADER_H
#define SHADER_H

#include <gl/glew.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint programID;

    Shader(const std::string &vertexFilePath, const std::string &fragmentFilePath);
    ~Shader();

    void use() const;

    GLint getUniformLoc(const std::string &name) const;

private:
    static bool logShaderError(const GLuint &shaderID);
    static bool logProgramError(const GLuint &programID);

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

#endif //SHADER_H
