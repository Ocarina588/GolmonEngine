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

const float PI = 3.14159265359;
const float F0 = 0.04; // Base reflectivity for non-metals

// Fresnel-Schlick approximation
float FresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX Normal Distribution Function (NDF)
float GGX_D(float alpha, float cosThetaH) {
    float alpha2 = alpha * alpha;
    float cosThetaH2 = cosThetaH * cosThetaH;
    return alpha2 / (PI * pow((cosThetaH2 * (alpha2 - 1.0) + 1.0), 2.0));
}

// Geometry function (Smith's method)
float GGX_G(float alpha, float cosTheta) {
    return 2.0 * cosTheta / (cosTheta + sqrt(alpha * alpha + (1.0 - alpha * alpha) * cosTheta * cosTheta));
}

// Compute the specular BRDF term
vec3 SpecularBRDF(vec3 omega_i, vec3 omega_o, vec3 normal, float metallic, float alpha) {
    vec3 h = normalize(omega_i + omega_o);  // Halfway vector
    float cosThetaH = dot(h, normal);

    // Microfacet distribution function
    float D = GGX_D(alpha, cosThetaH);
    // Fresnel term
    float F = FresnelSchlick(dot(omega_i, h), F0);
    // Geometry term
    float G = GGX_G(alpha, dot(omega_i, normal)) * GGX_G(alpha, dot(omega_o, normal));
    // Specular BRDF formula
    return vec3(D * F * G) / (4.0 * dot(omega_i, normal) * dot(omega_o, normal));
}

// Compute the diffuse BRDF term (Lambertian with retroreflection bias)
vec3 DiffuseBRDF(vec3 omega_o, vec3 normal, vec3 albedo, float metallic) {
    float cosThetaO = max(dot(omega_o, normal), 0.0);
    return (1.0 - metallic) * albedo / PI * cosThetaO;
}

void main() {
	vec3  albedo    = pc.index_albedo    != 42 ? texture(textures[pc.index_albedo - 1], uv).rgb : vec3(1.0, 1.0, 1.0);
	vec3  normal    = pc.index_normal    != 42 ? normalize(TBN * (texture(textures[pc.index_normal - 1], uv).rgb * 2.0 - 1.0)) : normalize(v_normal);
	vec3  emissive  = pc.index_emissive  != 42 ? texture(textures[pc.index_emissive - 1], uv).rgb : vec3(0.0, 0.0, 0.0);
	float occlusion = pc.index_occlusion != 42 ? texture(textures[pc.index_occlusion - 1], uv).r : 1.0;
	float metallic  = pc.index_metallic  != 42 ? texture(textures[pc.index_metallic - 1], uv).g : 0.0;
	float roughness = pc.index_metallic  != 42 ? texture(textures[pc.index_metallic - 1], uv).b : 0.5;
   
    if (pc.index_debug == 42)
        outColor = vec4(0, 0, 0, 1);
    else if (pc.index_debug > 0) {
		if (pc.index_debug == pc.index_occlusion)
			outColor = vec4(occlusion, occlusion, occlusion, 1);
		else if (pc.index_debug == pc.index_metallic)
			outColor = vec4(metallic, metallic, metallic, 1);
		else if (pc.index_debug == pc.index_roughness)
			outColor = vec4(roughness, roughness, roughness, 1);
		else
			outColor = vec4(texture(textures[pc.index_debug - 1], uv).rgb, 1);
    }
    if (pc.index_debug > 0)
        return;
    
    if (pc.light_equation == 1) {
        // Compute the outgoing direction (view vector, assuming camera is at view_pos)
        vec3 omega_o = normalize(view_pos - frag_pos); // Camera/view vector
        vec3 omega_i = normalize(light_pos - frag_pos); // Light vector
        //DISNEY BRDF
        vec3 diffuse = DiffuseBRDF(omega_o, normal, albedo, metallic);
        vec3 specular = SpecularBRDF(omega_i, omega_o, normal, metallic, roughness);

        vec3 final_result = diffuse + specular + emissive;
        outColor = vec4(final_result * occlusion, 1.0);
    }
    else {
        // PHONG MODEL
        vec3 light_dir = normalize(light_pos - frag_pos);
        vec3 view_dir = normalize(view_pos - frag_pos);
        vec3 reflect_dir = reflect(-light_dir, normal);

        float ambient_strength = 0.4f;
        float specular_strength = 0.5f;
        vec3 light_color = vec3(1.f, 1.f, 1.f);
        
        vec3 ambient = ambient_strength * light_color + emissive;

        float diffuse_angle = max(0, dot(light_dir, normal));
        vec3 diffuse = diffuse_angle * light_color;

        float specular_angle = pow(max(dot(view_dir, reflect_dir), 0.0), 10);
        vec3 specular = specular_strength * specular_angle * light_color;
        
        outColor = vec4((ambient + diffuse + specular) * albedo, 1.0f);
    }
}

/*
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(t_normal, uv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(frag_pos);
    vec3 Q2 = dFdy(frag_pos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(v_normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo = pow(texture(t_albedo, uv).rgb, vec3(2.2));
    float metallic = texture(t_metallic, uv).g;
    float roughness = texture(t_metallic, uv).b;
    float ao = texture(t_occlusion, uv).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(view_pos - frag_pos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 1; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(light_pos - frag_pos);
        vec3 H = normalize(V + L);
        float distance = length(light_pos - frag_pos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(1.f, 1.f, 1.f) * attenuation * 2;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color + texture(t_emissive, uv).rgb, 1.0);
}
*/