#pragma once
#include <streamline/metaprogramming/empty_t.hpp>

#include "sirius/core/error.hpp"


namespace acma::timeline {
	template<typename T>
	struct setup {
		constexpr result<sl::empty_t> operator()(auto const&, auto&) const noexcept {
			return {};
		}
	};
}