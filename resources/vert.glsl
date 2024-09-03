#version 330 core
in vec3 aPos;
in vec3 aColour;

out vec4 vertexColor;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    vertexColor = vec4(aColour, 1.0);
}