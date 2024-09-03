//
// Created by fripe on 03/09/2024.
//

#include "Shader.h"
#include <fstream>
#include <vector>
#include <SDL.h>

bool Shader::logShaderError(const GLuint &shaderID) {
    GLint result = GL_FALSE;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    if (result != GL_FALSE)
        return false;

    GLint infoLogLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    char infoLog[infoLogLength];
    glGetShaderInfoLog(shaderID, infoLogLength, nullptr, infoLog);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader compilation failed: %s", infoLog);
    return true;
}

bool Shader::logProgramError(const GLuint &programID) {
    GLint result = GL_FALSE;
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if (result != GL_FALSE)
        return false;

    GLint infoLogLength;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    char infoLog[infoLogLength];
    glGetProgramInfoLog(programID, infoLogLength, nullptr, infoLog);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program linking failed: %s", infoLog);
    return true;
}


std::string Shader::readShaderFile(const std::string &filePath) {
    std::ifstream file(filePath);
    printf("Reading file: %s\n", filePath.c_str());

    std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return fileContents;
}

Shader::Shader(const std::string &vertexFilePath, const std::string &fragmentFilePath) {
    const auto vertShaderSrc = readShaderFile(vertexFilePath);
    const auto fragShaderSrc = readShaderFile(fragmentFilePath);

    const GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexSource = vertShaderSrc.c_str();
    glShaderSource(vertShaderID, 1, &vertexSource, nullptr);
    glCompileShader(vertShaderID);
    if (logShaderError(vertShaderID))
        return;

    const GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentSource = fragShaderSrc.c_str();
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
