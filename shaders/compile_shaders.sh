glslc -fshader-stage=vertex vertex.glsl -o vertex.spv --target-spv=spv1.4
glslc -fshader-stage=fragment fragment.glsl -o fragment.spv --target-spv=spv1.4
glslc -fshader-stage=vertex cube_map.vert.glsl -o cube_map.vert.spv --target-spv=spv1.4
glslc -fshader-stage=fragment cube_map.frag.glsl -o cube_map.frag.spv --target-spv=spv1.4

