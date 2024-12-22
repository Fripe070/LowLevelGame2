#ifndef GRAPHICS_SHADER_H
#define GRAPHICS_SHADER_H
#include <string>
#include <vector>
#include <gl/glew.h>

#include "shader_program.h"

namespace Engine {
    class GraphicsShader : public ShaderProgram {
    public:
        GraphicsShader(const std::string &vertexFilePath, const std::string &fragmentFilePath)
            : ShaderProgram({
                  {vertexFilePath, GL_VERTEX_SHADER},
                  {fragmentFilePath, GL_FRAGMENT_SHADER}
              }) {};
        GraphicsShader(const std::string &vertexFilePath, const std::string &geometryFilePath, const std::string &fragmentFilePath)
            : ShaderProgram({
                {vertexFilePath, GL_VERTEX_SHADER},
                {geometryFilePath, GL_GEOMETRY_SHADER},
                {fragmentFilePath, GL_FRAGMENT_SHADER}
            }) {};
        /*!
         * @brief Construct a shader program from multiple files
         * @param filePaths A vector of pairs of file paths and shader types (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER...)
         */
        explicit GraphicsShader(const std::vector<std::pair<std::string, unsigned int>> &filePaths)
            : ShaderProgram(filePaths) {};
    };
}

#endif
