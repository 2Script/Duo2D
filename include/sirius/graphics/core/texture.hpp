#pragma once
#include <vector>
#include <streamline/numeric/int.hpp>

#include "sirius/graphics/core/texture_info.hpp"
#include "sirius/graphics/core/texture_view.hpp"


namespace acma {
	struct texture : texture_info {
		std::vector<sl::byte> bytes;

	public:
		constexpr operator texture_view() const& noexcept {
			return {*this, {bytes.data(), bytes.size()}};
		};
	};
}