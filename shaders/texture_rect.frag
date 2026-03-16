#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 1, binding = 0) uniform sampler samplers[];
layout(set = 2, binding = 0) uniform texture2D textures[];

layout(location = 0) in vec2 uv_in;

layout(location = 0) out vec4 color_out;



void main() {
    color_out = texture(nonuniformEXT(sampler2D(textures[0], samplers[0])), uv_in);
}