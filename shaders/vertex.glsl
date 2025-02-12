#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 light_pos;
    vec4 view_pos;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 Out_normal;
layout(location = 1) out vec3 Out_tangent;
layout(location = 2) out vec3 Out_bitangent;
layout(location = 3) out vec2 Out_uv;
layout(location = 4) out vec3 Out_light_pos;
layout(location = 5) out vec3 Out_frag_pos;
layout(location = 6) out vec3 Out_view_pos;
layout(location = 7) out mat3 Out_TBN;

void main() {
    mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));

    Out_tangent = normalize(normal_matrix * tangent);
    Out_normal = normalize(normal_matrix * normal);
    Out_tangent = normalize(Out_tangent - dot(Out_tangent, Out_normal) * Out_normal);
    Out_bitangent = normalize(cross(Out_normal, Out_tangent));

    //vec3 T = normalize(vec3(ubo.model * vec4(Out_tangent, 0.0)));
    //vec3 B = normalize(vec3(ubo.model * vec4(Out_bitangent, 0.0)));
    //vec3 N = normalize(vec3(ubo.model * vec4(Out_normal, 0.0)));
    Out_TBN = mat3(Out_tangent, Out_bitangent, Out_normal);


    Out_uv = uv;
    Out_light_pos = ubo.light_pos.rgb;
    Out_frag_pos = vec3(ubo.model * vec4(pos, 1.0));
    Out_view_pos = ubo.view_pos.rgb;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
}