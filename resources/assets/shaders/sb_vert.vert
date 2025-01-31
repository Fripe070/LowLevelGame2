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
    vec4 pos = vec4(projection * mat4(mat3(view)) * vec4(iPos, 1.0));
    pos.z = 0.0; // Always at "max" depth (we use reverse-z, so it's 0 not 1)
    gl_Position = pos;
}

