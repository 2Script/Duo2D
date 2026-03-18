#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/drawable.hpp"
#include "sirius/shaders/rect.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace acma::test {
	struct styled_rect : public acma::drawable {
        constexpr static auto vert_shader_data = std::to_array(acma::shaders::rect::vert);
        constexpr static auto frag_shader_data = std::to_array(acma::shaders::rect::frag);
	public:
		constexpr static auto buffers = buffer_key_sequence<
			::buffer_id::draw_constants,
			::buffer_id::draw_commands,
			::buffer_id::counts,

			::buffer_id::rectangle_indices,
			::buffer_id::positions
		>;
	public:
		constexpr static sl::array<1, sl::key_value_pair<buffer_key_t, buffer_key_t>> draw_buffers{{
			{::buffer_id::draw_commands, ::buffer_id::counts}
		}};

	public:
		constexpr static VkIndexType index_type = VK_INDEX_TYPE_UINT16;
	};
}