#version 450

layout(location = 0) in vec2 uv_in;

layout(location = 0) out vec4 color_out;



void main() {
    color_out = vec4(uv_in, 0.0, 1.0);
}