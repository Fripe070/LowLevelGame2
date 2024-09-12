#version 330 core
in vec3 aPos;
in vec3 aColour;
in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = vec4(aColour, 1.0);
    texCoord = aTexCoord;
}