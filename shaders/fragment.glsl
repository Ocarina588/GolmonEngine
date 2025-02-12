#version 450

layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec3 v_tangent;
layout(location = 2) in vec3 v_bitangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 light_pos;
layout(location = 5) in vec3 frag_pos;
layout(location = 6) in vec3 view_pos;
layout(location = 7) in mat3 TBN;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D t_albedo;
layout(set = 0, binding = 2) uniform sampler2D t_normal;
layout(set = 0, binding = 3) uniform sampler2D t_metallic;
layout(set = 0, binding = 4) uniform sampler2D t_emissive;
layout(set = 0, binding = 5) uniform sampler2D t_occlusion;

vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
float ambient_strength = 0.05;
float specular_strength = 0.5;

void main() {

    vec3 color  = texture(t_albedo, uv).rgb;
    vec3 normal = texture(t_normal, uv).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    normal = v_normal;
    
    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    //AMBIENT
    vec3 ambient = ambient_strength * light_color;
    
    //DIFFUSE
    float diffuse_angle = max(0, dot(light_dir, normal));
    vec3 diffuse = diffuse_angle * light_color;

    //SPECULAR
    float specular_angle = pow(max(dot(view_dir, reflect_dir), 0.0), 10);
    vec3 specular = specular_strength * specular_angle * light_color;

    //RESULT
    vec3 final_result = (ambient + diffuse + specular) * color;

    outColor = vec4(final_result, 1.0f);
}