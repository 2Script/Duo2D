#pragma once
#include <streamline/containers/version.hpp>
#include <streamline/numeric/int.hpp>


namespace acma {
	using version = sl::generic::version<sl::uint16_t>;
}

namespace acma {
	constexpr version engine_version{0, 0, 1}; 
}