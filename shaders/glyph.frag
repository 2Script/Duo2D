#version 450
#extension GL_EXT_nonuniform_qualifier : require


layout(binding = 1) uniform sampler2DArray sampled_textures[];

layout(location = 0) in vec4 color_in;
layout(location = 1) flat in uint font_texture_idx_in;
layout(location = 2) in vec3 uv_in;
layout(location = 3) in vec2 size_in;

layout(location = 0) out vec4 color_out;


float median(vec3 value){
    return max(min(value.r, value.g), min(max(value.r, value.g), value.b));
}

void main() {
    if(font_texture_idx_in >= 65535) {
        color_out = color_in;
        return;
    }
    
    const uvec2 texture_extent = textureSize(nonuniformEXT(sampled_textures[font_texture_idx_in]), 0).xy;
    const vec4 tex = texture(nonuniformEXT(sampled_textures[font_texture_idx_in]), uv_in);
    const vec2 texture_ratio = size_in/texture_extent;
    //color_out.w = color_in.w * (1 - tex.w) + tex.w;
    //color_out.xyz = color_in.xyz * color_in.w * (1 - tex.w) + tex.xyz * tex.w;
    //if(color_out.w > 0) color_out.xyz /= color_out.w;
    const float sd = median(tex.rgb);
    const float px_range = 4.0;
    const float screen_px_dist = min(texture_ratio.x, texture_ratio.y) * px_range * (sd - 0.5);
    const float opacity = clamp(screen_px_dist + 0.5, 0.0, 1.0);
    const vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    color_out = vec4(white.rgb * opacity, 1.0);
    //color_out = mix(color_in, white, opacity);
}