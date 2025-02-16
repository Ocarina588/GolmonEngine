#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 light_pos;
layout(location = 2) in vec3 frag_pos;
layout(location = 3) in vec3 view_pos;
layout(location = 4) in mat3 TBN;
layout(location = 7) in vec3 v_normal;

layout(location = 8) flat in uint index_albedo;
layout(location = 9) flat in uint index_normal;
layout(location = 10) flat in uint index_metallic;
layout(location = 11) flat in uint index_emissive;
layout(location = 12) flat in uint index_occlusion;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D t_albedo;
layout(set = 0, binding = 2) uniform sampler2D t_normal;
layout(set = 0, binding = 3) uniform sampler2D t_metallic;
layout(set = 0, binding = 4) uniform sampler2D t_emissive;
layout(set = 0, binding = 5) uniform sampler2D t_occlusion;

//layout(set = 0, binding = 6) uniform sampler2D t_background;

vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
float ambient_strength = 0.5;
float specular_strength = 0.5;

void main() 
{
    vec3 albedo = vec3(1.f, 1.f, 1.f);
	vec3 emissive = vec3(0.f, 0.f, 0.f);
	vec3 occlusion = vec3(1.f, 1.f, 1.f);
    vec3 normal = v_normal;
	float metallic = 0.0;
	float roughness = 0.0;

	if (index_albedo > 0)
		albedo = texture(t_albedo, uv).rgb;
	if (index_normal > 0)
        normal = normalize(TBN * (texture(t_normal, uv).rgb * 2.0 - 1.0));
	if (index_emissive > 0)
		emissive = texture(t_emissive, uv).rgb;
    if (index_metallic > 0) {
        metallic = texture(t_metallic, uv).g;
		roughness = texture(t_metallic, uv).b;
    }
	if (index_occlusion > 0)
		occlusion = texture(t_occlusion, uv).rgb;

    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    //AMBIENT
    vec3 ambient = ambient_strength * albedo * light_color + emissive;
    
    //DIFFUSE
    float diffuse_angle = max(0, dot(light_dir, normal));
    vec3 diffuse = diffuse_angle * light_color;

    //SPECULAR
    float specular_angle = pow(max(dot(view_dir, reflect_dir), 0.0), 10);
    vec3 specular = specular_strength * specular_angle * light_color;

    //RESULT
    vec3 final_result = (ambient + diffuse + specular) * albedo;

    vec3 dir = normalize(view_pos - vec3(0.0, 0.0, 0.0));
    // Convert direction to spherical coordinates
    float u = atan(dir.z, dir.x) / (2.0 * 3.14159265359) + 0.5;
    float v = asin(dir.y) / 3.14159265359 + 0.5;

    // Sample the environment texture

    outColor = vec4(final_result, 1.0f);
}