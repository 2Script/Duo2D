#version 450

layout(binding = 0) uniform UBO {
    uvec2 swap_chain_extent;
} ubo;

//per-instance attributes
layout(location = 0) in uint char;
layout(location = 1) in uint size_in;
layout(location = 2) in vec2 pos_in;
layout(location = 3) in uvec4 color_in;

//per-instance texture indices
layout(location = 4) in uint font_texture_idx_in;

layout(location = 0) out vec4 color_out;
layout(location = 1) out uint font_texture_idx_out;
layout(location = 2) out vec3 uv_out;
layout(location = 3) out uint size_out;

void main() {
    font_texture_idx_out = font_texture_idx_in;
    if(char == 0) {
        gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        uv_out = vec3(0, 0, 0);
        return;
    }
    

    const uvec2 uv = uvec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
    const vec2 pos = pos_in + (size_in * uv);

    gl_Position = vec4(((pos * 2)/ubo.swap_chain_extent) - 1, 0.0, 1.0);
    uv_out = vec3(uv, char - 32);
    size_out = size_in;
    color_out = color_in/255.0;
}