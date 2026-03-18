#pragma once
#include <streamline/numeric/int.hpp>


namespace acma {
	using asset_group_t = sl::uint8_t;
}

namespace acma {
	namespace asset_group {
	enum : asset_group_t {
		sampler,
		image,
		uniform,

		num_asset_groups,
	};
	}
}