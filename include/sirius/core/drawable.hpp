#pragma once
#include <streamline/containers/array.hpp>
#include <streamline/containers/key_value_pair.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>
#include <streamline/numeric/int.hpp>

#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/buffer_key_t.hpp"


namespace acma {
	struct drawable {
		consteval static sl::uint32_t max_draw_count() noexcept { return static_cast<sl::uint32_t>(-1); }
	public:
		constexpr static auto buffers = buffer_key_sequence<>;
		constexpr static auto asset_heaps = asset_heap_key_sequence<>;
	public:
		constexpr static auto uniform_buffer_order = buffer_key_sequence<>;
		constexpr static sl::array<0, sl::key_value_pair<buffer_key_t, buffer_key_t>> draw_buffers{};
	};
}