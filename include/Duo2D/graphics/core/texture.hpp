#pragma once
#include <vector>
#include <streamline/numeric/int.hpp>

#include "Duo2D/graphics/core/texture_info.hpp"
#include "Duo2D/graphics/core/texture_view.hpp"


namespace d2d {
	struct texture : texture_info {
		std::vector<sl::byte> bytes;

	public:
		constexpr operator texture_view() const& noexcept {
			return {*this, {bytes.data(), bytes.size()}};
		};
	};
}