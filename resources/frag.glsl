#version 330 core
in vec4 vertexColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture1;


void main() {
//    FragColor = vertexColor;
//    FragColor = vec4(texCoord, 0.0, 1.0);
    FragColor = texture(texture1, texCoord);
}