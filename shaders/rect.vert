#version 450

layout(binding = 0) uniform UBO {
    uvec2 extent;
} ubo;

layout(push_constant) uniform Transform {
    vec2 scale;
    mat2x2 rotation;
    vec2 translation;
} xfrm;

//per-instance input
layout(location = 0) in vec2 pos_in;
layout(location = 1) in vec2 size_in;
layout(location = 2) in uvec4 color_in;

//layout(location = 3) in uint index;

layout(location = 0) out vec4 color_out;

void main() {
    const vec2 points[4] = {pos_in, vec2(pos_in[0] + size_in[0], pos_in[1]), pos_in + size_in, vec2(pos_in[0], pos_in[1] + size_in[1])};
    gl_Position = vec4(points[gl_VertexIndex]/ubo.extent, 0.0, 1.0);
    //vec4 color = uvec4((color_in >> 24) & 0xff, (color_in >> 16) & 0xff, (color_in >> 8) & 0xff, (color_in >> 0) & 0xff)/255.0;
    //vec4 colors[4] = {color, vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0)};
    //color_out = colors[gl_VertexIndex];
    //color_out = uvec4((color_in >> 24) & 0xff, (color_in >> 16) & 0xff, (color_in >> 8) & 0xff, (color_in >> 0) & 0xff)/255.0;
    color_out = color_in/255.0;
}