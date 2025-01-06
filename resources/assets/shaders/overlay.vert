#version 410 core
out vec2 TexCoord;
in vec3 iPos;

void main() {
    TexCoord = iPos.xy * 0.5 + 0.5;
    gl_Position = vec4(iPos, 1.0);
}