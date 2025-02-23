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

layout(location = 0) out vec3 Out_frag_pos;

void main() 
{
    mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));

    vec3 T = normalize(normal_matrix * tangent);
    vec3 B = normalize(normal_matrix * bitangent);
    vec3 N = normalize(normal_matrix * normal);

    Out_frag_pos = vec3(vec4(pos, 1.0));


    gl_Position = ubo.proj * mat4(mat3(ubo.view)) * vec4(pos, 1.0);
}



