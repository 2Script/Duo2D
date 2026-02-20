#pragma once
#include <streamline/metaprogramming/empty_t.hpp>

#include "Duo2D/core/error.hpp"


namespace d2d::timeline {
	template<typename T>
	struct setup {
		constexpr result<sl::empty_t> operator()(auto const&) const noexcept {
			return {};
		}
	};
}