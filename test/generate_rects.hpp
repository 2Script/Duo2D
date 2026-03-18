#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/dispatchable.hpp"
#include "sirius/shaders/rect.hpp"
#include "sirius/shaders/generate_rects.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace acma::test {
	struct generate_rects : public acma::dispatchable {
        constexpr static auto comp_shader_data = std::to_array(acma::shaders::generate_rects::comp);
	public:
		constexpr static auto buffers = acma::buffer_key_sequence<
			::buffer_id::compute_constants,
			::buffer_id::dispatch_commands,
			
			::buffer_id::counts,

			::buffer_id::draw_commands,
			::buffer_id::positions,

			::buffer_id::offset
		>;
		constexpr static auto uniform_buffer_order = acma::buffer_key_sequence<
			::buffer_id::offset
		>;
		constexpr static auto asset_heaps = acma::asset_heap_key_sequence<
			::asset_heap_id::compute
		>;
	public:
		constexpr static sl::array<1, buffer_key_t> dispatch_buffers{{
			::buffer_id::dispatch_commands
		}};
	};
}