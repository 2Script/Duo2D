#pragma once
#include <streamline/numeric/int.hpp>

#include "Duo2D/graphics/core/texture_info.hpp"


namespace d2d::vk {
	struct texture_data_info : texture_info {
		sl::uoffset_t offset;
		sl::size_t size;
	};
}