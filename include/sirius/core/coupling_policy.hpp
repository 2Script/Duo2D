#pragma once
#include <streamline/numeric/int.hpp>


namespace acma{
	using coupling_policy_t = sl::uint_fast8_t;
}

namespace acma {
	namespace coupling_policy {
	enum : coupling_policy_t {
		decoupled,
		coupled,

		num_coupling_policies
	};
	}
}