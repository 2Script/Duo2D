#pragma once
#include <streamline/numeric/int.hpp>

namespace d2d {
	struct drawable {
		consteval static sl::uint32_t max_draw_count() noexcept { return static_cast<sl::uint32_t>(-1); }
	};
}