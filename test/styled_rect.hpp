#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/core/drawable.hpp"
#include "Duo2D/shaders/rect.hpp"

#include "./buffer_config_table.hpp"

namespace d2d::test {
	struct styled_rect : public d2d::drawable {
        constexpr static auto vert_shader_data = std::to_array(d2d::shaders::rect::vert);
        constexpr static auto frag_shader_data = std::to_array(d2d::shaders::rect::frag);
		constexpr static auto buffers = sl::integer_sequence<buffer_key_t, 
			::buffer_id::draw_constants,
			::buffer_id::draw_commands,
			::buffer_id::counts,

			::buffer_id::rectangle_indices,
			::buffer_id::positions
		>;

		constexpr static auto draw_buffers = sl::array<1, sl::key_value_pair<buffer_key_t, buffer_key_t>> {{
			{::buffer_id::draw_commands, ::buffer_id::counts}
		}};

	public:
		constexpr static VkIndexType index_type = VK_INDEX_TYPE_UINT16;
	};
}