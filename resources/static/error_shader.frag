#version 410 core
out vec4 oFragColor;

float hash(float x) {
    return fract(sin(x) * 78657.11909);
}

void main()
{
    float id = float(gl_PrimitiveID);
    float r = hash(id);
    float g = hash(id+1);
    float b = hash(id+2);
    vec3 col = (vec3(r, g, b) - 0.5) * 0.75 + 0.5;
    oFragColor = vec4(col, 1.0);
}
