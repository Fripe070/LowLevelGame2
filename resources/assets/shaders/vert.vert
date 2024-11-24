#version 330 core
out vec4 VertexColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

in vec3 iPos;
in vec3 iNormal;
in vec2 iTexCoord;
in vec4 iColor;

uniform mat4 model;
uniform mat3 mTransposed;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(iPos, 1.0));
    Normal = mTransposed * iNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);

    TexCoord = iTexCoord;
    VertexColor = iColor;
}
