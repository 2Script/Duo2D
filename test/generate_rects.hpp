#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/shaders/rect.hpp"
#include "Duo2D/shaders/generate_rects.hpp"

#include "./buffer_config_table.hpp"

namespace d2d::test {
	struct generate_rects {
        constexpr static auto comp_shader_data = std::to_array(d2d::shaders::generate_rects::comp);
		constexpr static auto buffers = d2d::buffer_key_sequence<
			::buffer_id::compute_constants,
			::buffer_id::dispatch_commands,
			
			::buffer_id::counts,

			::buffer_id::draw_commands,
			::buffer_id::positions
		>;
		
		constexpr static auto dispatch_buffers = sl::array<1, buffer_key_t>{{
			::buffer_id::dispatch_commands
		}};
	};
}