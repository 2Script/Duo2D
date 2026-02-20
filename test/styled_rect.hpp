#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/shaders/rect.hpp"

#include "./resource_table.hpp"

namespace d2d::test {
	struct styled_rect {
        constexpr static auto vert_shader_data = std::to_array(d2d::shaders::rect::vert);
        constexpr static auto frag_shader_data = std::to_array(d2d::shaders::rect::frag);
		constexpr static auto buffers = sl::integer_sequence<resource_key_t, 
			::resource_id::push_constants,
			::resource_id::draw_commands,

			::resource_id::rectangle_indices,
			::resource_id::positions
		>;

		consteval static sl::size_t draw_count() noexcept { return 1; }

	public:
		constexpr static VkIndexType index_type = VK_INDEX_TYPE_UINT16;
	};
}