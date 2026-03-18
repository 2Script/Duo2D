#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/drawable.hpp"
#include "sirius/shaders/texture_rect.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace acma::test {
	struct texture_rect : public acma::drawable {
        constexpr static auto vert_shader_data = std::to_array(acma::shaders::texture_rect::vert);
        constexpr static auto frag_shader_data = std::to_array(acma::shaders::texture_rect::frag);
	public:
		constexpr static auto buffers = buffer_key_sequence<
			::buffer_id::draw_constants,
			::buffer_id::single_instance_draw_command,
			::buffer_id::counts,

			::buffer_id::rectangle_indices
		>;
		constexpr static auto asset_heaps = acma::asset_heap_key_sequence<
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