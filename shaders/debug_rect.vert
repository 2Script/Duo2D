#version 450

layout(binding = 0) uniform UBO {
    uvec2 extent;
} ubo;

//per-vertex input
layout(location = 0) in vec2 pos_in;
layout(location = 1) in uvec4 color_in;

layout(location = 2) in vec2 scale;
layout(location = 3) in mat2x2 rotate;
layout(location = 5) in vec2 translate;
layout(location = 6) in uint border_width;


layout(location = 0) out vec4 color_out;
layout(location = 1) out uvec2 debug;

void main() {
    gl_Position = vec4(pos_in/ubo.extent, 0.0, 1.0);
    debug = ubo.extent;
    color_out = color_in/255.0;
}