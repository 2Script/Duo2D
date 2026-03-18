#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/graphics/core/texture_info.hpp"


namespace acma::vk {
	struct texture_data_info : texture_info {
		sl::uoffset_t offset;
		sl::size_t size;
	};
}