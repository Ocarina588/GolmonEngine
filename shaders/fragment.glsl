#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;           // Texture coordinates
layout(location = 1) in vec3 light_pos;    // Light position
layout(location = 2) in vec3 frag_pos;     // Fragment position in world space
layout(location = 3) in vec3 view_pos;     // Camera/view position
layout(location = 4) in mat3 TBN;          // Tangent, Bitangent, Normal matrix
layout(location = 7) in vec3 v_normal;     // Vertex normal

layout(set = 0, binding = 1) uniform sampler2D background;
layout(set = 0, binding = 2) uniform sampler2D textures[];

layout(push_constant) uniform PushConstants {
    uint index_albedo;  
    uint index_normal;
    uint index_metallic;
    uint index_roughness;
    uint index_emissive;
    uint index_occlusion;
    uint index_debug;
    uint light_equation;
} pc;

void render_equation_disney_BRDF(void);
void phong(void);

vec3  albedo = pc.index_albedo != 42 ? texture(textures[pc.index_albedo - 1], uv).rgb : vec3(1.0, 1.0, 1.0);
vec3  normal = pc.index_normal != 42 ? normalize(TBN * (texture(textures[pc.index_normal - 1], uv).rgb * 2.0 - 1.0)) : normalize(v_normal);
vec3  emissive = pc.index_emissive != 42 ? texture(textures[pc.index_emissive - 1], uv).rgb : vec3(0.0, 0.0, 0.0);
float occlusion = pc.index_occlusion != 42 ? texture(textures[pc.index_occlusion - 1], uv).g : 1.0;
float metallic = pc.index_metallic != 42 ? texture(textures[pc.index_metallic - 1], uv).b : 0.0;
float roughness = pc.index_roughness != 42 ? texture(textures[pc.index_metallic - 1], uv).g : 0.5;

vec3 sRGBToLinear(vec3 color) {
    return mix(
        color / 12.92,
        pow((color + 0.055) / 1.055, vec3(2.4)),
        step(vec3(0.04045), color)
    );
}

vec3 linearToSRGB(vec3 color) {
    return mix(
        color * 12.92,
        1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055,
        step(vec3(0.0031308), color)
    );
}

void print_debug() {
    if (pc.index_debug == 1)
        outColor = vec4(albedo.rgb, 1.0);
    else if (pc.index_debug == 2)
        outColor = vec4(normal.rgb, 1.0);
    else if (pc.index_debug == 3)
        outColor = vec4(metallic, metallic, metallic, 1.0);
    else if (pc.index_debug == 4)
        outColor = vec4(roughness, roughness, roughness, 1.0);
    else if (pc.index_debug == 5)
        outColor = vec4(emissive.rgb, 1.0);
    else if (pc.index_debug == 6)
        outColor = vec4(occlusion, occlusion, occlusion, 1.0);
    else
        outColor = vec4(1, 0.5, 0.5, 1);
}

const float PI = 3.14159265359;
vec3 F0 = vec3(0.04);// mix(vec3(0.04), albedo, metallic);

struct brdf_struct {
	vec3 N;
	vec3 L;
	vec3 V;
	vec3 H;
	float NdotL;
	float NdotV;
	float NdotH;
	float VdotH;
    float alpha;
};

float GGX(brdf_struct info)
{
    float epsilon = 1e-6;
	float alpha2 = info.alpha * info.alpha;
	float NdotH2 = info.NdotH * info.NdotH;
	float denominator = NdotH2 * (alpha2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator + epsilon;
    return max(0, alpha2 / denominator);
}

float G_SMITH(brdf_struct info)
{
	float k = info.alpha * sqrt(2.0 / PI);
	float g1 = info.NdotV / (info.NdotV * (1.0 - k) + k);
	float g2 = info.NdotL / (info.NdotL * (1.0 - k) + k);
    return max(0, min(1, g1 * g2));
}

vec3 f_specular(brdf_struct info)
{
    return max(vec3(0), F0 + (1.0 - F0) * pow(clamp(1.0 - info.VdotH, 0, 1), 5.0));
}

void main() {
    F0 = mix(vec3(0.04), albedo, metallic);
    albedo = sRGBToLinear(albedo);

    if (pc.index_debug > 0)
        print_debug();
    else if (pc.light_equation == 0)
        render_equation_disney_BRDF();
    else
        phong();

    outColor.rgb = linearToSRGB(outColor.rgb);
}

void render_equation_disney_BRDF(void)
{
    F0 = mix(vec3(0.5), albedo, metallic);
    brdf_struct info;
    info.N = normalize(normal);
    info.L = normalize(light_pos);
    info.V = normalize(view_pos - frag_pos);
    info.H = normalize(info.L + info.V);
    info.alpha = roughness * roughness;
    info.NdotH = max(dot(info.N, info.H), 0.0);
    info.NdotL = max(dot(info.N, info.L), 0.0);
    info.NdotV = max(dot(info.N, info.V), 0.0);
    info.VdotH = max(dot(info.V, info.H), 0.0);

    vec3 diffuse = albedo / PI;
    vec3 specular = vec3((GGX(info) * G_SMITH(info) * f_specular(info)) / (4.0 * info.NdotL * info.NdotV));
    vec3 kS = vec3(f_specular(info));
    vec3 kD = vec3(1.0) - kS * (1.0 - metallic);
    vec3 BRDF = kD * diffuse + max(vec3(0), specular);
    vec3 incoming_light = vec3(1.0f, 1.0f, 1.0f);

    outColor.rgb = BRDF * incoming_light * info.NdotL;
	outColor.rgb += emissive;
    outColor.rgb *= occlusion;
}

void phong(void)
{
    // PHONG MODEL
    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    float ambient_strength = 0.1f;
    float specular_strength = 0.7f;
    vec3 light_color = vec3(1.f, 1.f, 1.f);

    vec3 ambient = ambient_strength * light_color + emissive;

    float diffuse_angle = max(0, dot(light_dir, normal));
    vec3 diffuse = diffuse_angle * light_color;

    float specular_angle = pow(max(dot(view_dir, reflect_dir), 0.0), 10);
    vec3 specular = specular_strength * specular_angle * light_color;

    outColor = vec4((ambient + diffuse + specular) * albedo, 1.0f);
}