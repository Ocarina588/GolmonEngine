#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outColor;
 // Light position
layout(location = 0) in vec3 frag_pos;     // Fragment position in world space
   // Vertex normal

layout(set = 0, binding = 1) uniform sampler2D textures[];


const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y *= -1;
    return uv;
}


void main() {

    outColor.rgb = texture(textures[0], SampleSphericalMap(normalize(frag_pos))).rgb;
}
