#version 330 core
in vec3 iPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = iPos;
    gl_Position = (projection * view * vec4(iPos, 1.0)).xyww;  // Trick to render at max depth
}

