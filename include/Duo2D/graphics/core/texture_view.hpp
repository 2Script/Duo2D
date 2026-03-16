#pragma once
#include <span>
#include <streamline/numeric/int.hpp>

#include "Duo2D/graphics/core/texture_info.hpp"


namespace d2d {
	struct texture_view : texture_info {
		std::span<const sl::byte> bytes;
	};
}