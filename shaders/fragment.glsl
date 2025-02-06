#version 450

layout(location = 0) in vec2 inColor;
layout(location = 0) out vec4 outColor;

vec3 colors[3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0}
};

void main() {
    outColor = vec4(colors[gl_PrimitiveID % 3], 1.0);
    outColor = vec4(inColor, 0.0, 1.0);
}