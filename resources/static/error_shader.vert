#version 410 core

in vec3 iPos;

layout(std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main() {
    gl_Position = projection * view * vec4(vec3(model * vec4(iPos, 1.0)), 1.0);
}
