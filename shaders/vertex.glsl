#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 Out_uv;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    Out_uv = uv;
}