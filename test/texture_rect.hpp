#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/core/drawable.hpp"
#include "Duo2D/shaders/texture_rect.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace d2d::test {
	struct texture_rect : public d2d::drawable {
        constexpr static auto vert_shader_data = std::to_array(d2d::shaders::texture_rect::vert);
        constexpr static auto frag_shader_data = std::to_array(d2d::shaders::texture_rect::frag);
	public:
		constexpr static auto buffers = buffer_key_sequence<
			::buffer_id::draw_constants,
			::buffer_id::single_instance_draw_command,
			::buffer_id::counts,

			::buffer_id::rectangle_indices
		>;
		constexpr static auto asset_heaps = d2d::asset_heap_key_sequence<
			::asset_heap_id::graphics
		>;
	public:
		constexpr static sl::array<1, sl::key_value_pair<buffer_key_t, buffer_key_t>> draw_buffers{{
			{::buffer_id::single_instance_draw_command, ::buffer_id::counts}
		}};

	public:
		constexpr static VkIndexType index_type = VK_INDEX_TYPE_UINT16;
	};
}