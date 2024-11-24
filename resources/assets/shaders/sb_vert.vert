#version 410 core
in vec3 iPos;

out vec3 TexCoords;

layout(std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    TexCoords = iPos;
    gl_Position = (projection * mat4(mat3(view)) * vec4(iPos, 1.0)).xyww;  // Trick to render at max depth
}

