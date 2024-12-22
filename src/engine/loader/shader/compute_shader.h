#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H
#include <string>
#include <vector>
#include <gl/glew.h>

#include "shader_program.h"

namespace Engine {
    class ComputeShader : public ShaderProgram {
    public:
        explicit ComputeShader(const std::string &computeFilePath)
            : ShaderProgram({
                {computeFilePath, GL_COMPUTE_SHADER}
            }) {};
    };
}

#endif
