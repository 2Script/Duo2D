#pragma once
#include <streamline/numeric/int.hpp>


namespace d2d{
	using coupling_policy_t = sl::uint_fast8_t;
}

namespace d2d {
	namespace coupling_policy {
	enum : coupling_policy_t {
		decoupled,
		coupled,

		num_coupling_policies
	};
	}
}