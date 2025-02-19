#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 light_pos;
    vec4 view_pos;
	uint index_albedo; 
	uint index_normal; 
	uint index_metallic; 
	uint index_emissive; 
	uint index_occlusion; 
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec2 Out_uv;
layout(location = 1) out vec3 Out_light_pos;
layout(location = 2) out vec3 Out_frag_pos;
layout(location = 3) out vec3 Out_view_pos;
layout(location = 4) out mat3 Out_TBN;
layout(location = 7) out vec3 Out_normal;
//layout(location = 8) out uint index_albedo;
//layout(location = 9) out uint index_normal;
//layout(location = 10) out uint index_metallic;
//layout(location = 11) out uint index_emissive;
//layout(location = 12) out uint index_occlusion;

void main() 
{
    mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));

    vec3 T = normalize(normal_matrix * tangent);
    vec3 B = normalize(normal_matrix * bitangent);
    vec3 N = normalize(normal_matrix * normal);

    Out_TBN = mat3(T, B, N);

    Out_normal = N;
    Out_uv = uv;
    Out_light_pos = ubo.light_pos.rgb;
    Out_frag_pos = vec3(ubo.model * vec4(pos, 1.0));
    Out_view_pos = ubo.view_pos.rgb;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
}



