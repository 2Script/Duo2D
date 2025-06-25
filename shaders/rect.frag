#version 450
#extension GL_EXT_nonuniform_qualifier : require


layout(binding = 1) uniform sampler2D sampled_textures[];

layout(location = 0) in vec4 color_in;
layout(location = 1) flat in uint background_texture_idx_in;
layout(location = 2) in vec2 uv_in;

layout(location = 0) out vec4 color_out;



void main() {
    if(background_texture_idx_in >= 65535) {
        color_out = color_in;
        return;
    }
    
    const uvec2 texture_extent = textureSize(nonuniformEXT(sampled_textures[background_texture_idx_in]), 0);
    //const vec2 uv = uv_in/texture_extents[nonuniformEXT(background_texture_idx_in)].value;
    const vec2 uv = uv_in/texture_extent;
    const vec4 tex = texture(nonuniformEXT(sampled_textures[background_texture_idx_in]), uv);
    color_out.w = color_in.w * (1 - tex.w) + tex.w;
    color_out.xyz = color_in.xyz * color_in.w * (1 - tex.w) + tex.xyz * tex.w;
    if(color_out.w > 0) color_out.xyz /= color_out.w;
    //color_out = vec4(uv, 0.0, 1.0);
}