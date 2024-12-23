#version 450

layout(binding = 0) uniform UBO {
    uvec2 extent;
} ubo;

//per-instance input
layout(location = 0) in vec2 pos_in;
layout(location = 1) in vec2 size_in;
layout(location = 2) in uvec4 color_in;

//per-instance attributes
layout(location = 3) in vec2 scale;
layout(location = 4) in mat2x2 rotate;
layout(location = 6) in vec2 translate;
layout(location = 7) in uint border_width;

layout(location = 0) out vec4 color_out;
layout(location = 1) out uvec2 debug;

void main() {
    const vec2 points[4] = {pos_in, vec2(pos_in[0] + size_in[0], pos_in[1]), pos_in + size_in, vec2(pos_in[0], pos_in[1] + size_in[1])};
    gl_Position = vec4(points[gl_VertexIndex]/ubo.extent, 0.0, 1.0);
    debug = ubo.extent;
    color_out = color_in/255.0;
}