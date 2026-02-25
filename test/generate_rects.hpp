#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/shaders/rect.hpp"
#include "Duo2D/shaders/generate_rects.hpp"

#include "./resource_table.hpp"

namespace d2d::test {
	struct generate_rects {
        constexpr static auto comp_shader_data = std::to_array(d2d::shaders::generate_rects::comp);
		constexpr static auto buffers = sl::integer_sequence<resource_key_t, 
			::resource_id::compute_constants,
			::resource_id::dispatch_commands,
			::resource_id::rectangle_limit,

			::resource_id::draw_commands,
			::resource_id::positions
		>;
	};
}