#pragma once
#include <span>
#include <streamline/numeric/int.hpp>

#include "sirius/graphics/core/texture_info.hpp"


namespace acma {
	struct texture_view : texture_info {
		std::span<const sl::byte> bytes;
	};
}