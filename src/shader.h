#ifndef SHADER_H
#define SHADER_H

#include <gl/glew.h>
#include <string>


class Shader {
public:
  GLuint programID;

  Shader(const std::string &vertexFilePath, const std::string &fragmentFilePath);
  ~Shader();
  void use() const;

private:
  static std::string readShaderFile(const std::string &filePath);
  static bool logShaderError(const GLuint &shaderID);
  static bool logProgramError(const GLuint &programID);
};



#endif //SHADER_H
