#version 460
#extension GL_EXT_buffer_reference : require



layout(buffer_reference, std430) readonly buffer PositionsBuffer{ 
	uvec2 positions[];
};


layout(std430, push_constant) uniform PushConstants {
	uvec2 swap_extent;
    PositionsBuffer buff;
} push_constants;

layout(location = 0) out vec2 uv_out;
layout(location = 1) out float blue_out;


void main() {
	const uint i = gl_InstanceIndex;
	//const uint i = gl_DrawID;
	const uvec2 size_in = uvec2(50, 25);
    const uvec2 uv_base = uvec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
	const uvec2 pos_in = push_constants.buff.positions[i];
    const vec2 pos = pos_in + (size_in * uv_base);
    //const vec2 uv = texture_bounds_pos_in + (texture_bounds_size_in * uv_base);
    
    gl_Position = vec4(((pos * 2)/push_constants.swap_extent) - 1, 0.0, 1.0);
	uv_out = uv_base;
	blue_out = i/(16.0 * 16.0);
    //color_out = color_in/255.0;

    //background_texture_idx_out = background_texture_idx_in;
    //uv_out = uv;
}