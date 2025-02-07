#version 450

layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 light_direction;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D t_albedo;
layout(set = 0, binding = 2) uniform sampler2D t_normal;
layout(set = 0, binding = 3) uniform sampler2D t_metallic;
layout(set = 0, binding = 4) uniform sampler2D t_emissive;
layout(set = 0, binding = 5) uniform sampler2D t_occlusion;

vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
float ambient_strength = 0.05;

void main() {

    vec3 color = texture(t_albedo, uv).rgb;
    vec3 normal = normalize(v_normal * texture(t_normal, uv).rgb);
    vec3 ambient = ambient_strength * light_color;
    float diffuse_strenght = max(0, dot(light_direction, normal));
    vec3 diffuse = diffuse_strenght * light_color;

    vec3 final_result = (ambient + diffuse) * color;

    outColor = vec4(final_result, 1.0f);
}