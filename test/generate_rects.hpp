#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "Duo2D/core/dispatchable.hpp"
#include "Duo2D/shaders/rect.hpp"
#include "Duo2D/shaders/generate_rects.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace d2d::test {
	struct generate_rects : public d2d::dispatchable {
        constexpr static auto comp_shader_data = std::to_array(d2d::shaders::generate_rects::comp);
	public:
		constexpr static auto buffers = d2d::buffer_key_sequence<
			::buffer_id::compute_constants,
			::buffer_id::dispatch_commands,
			
			::buffer_id::counts,

			::buffer_id::draw_commands,
			::buffer_id::positions,

			::buffer_id::offset
		>;
		constexpr static auto uniform_buffer_order = d2d::buffer_key_sequence<
			::buffer_id::offset
		>;
		constexpr static auto asset_heaps = d2d::asset_heap_key_sequence<
			::asset_heap_id::compute
		>;
	public:
		constexpr static sl::array<1, buffer_key_t> dispatch_buffers{{
			::buffer_id::dispatch_commands
		}};
	};
}