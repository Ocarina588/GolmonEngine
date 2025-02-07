#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 light_position;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 Out_normal;
layout(location = 1) out vec2 Out_uv;
layout(location = 2) out vec3 Out_light;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    Out_normal = normal;
    Out_uv = uv;
    Out_light = ubo.light_position;
}