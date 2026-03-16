#pragma once
#include <streamline/numeric/int.hpp>


namespace d2d {
	using asset_group_t = sl::uint8_t;
}

namespace d2d {
	namespace asset_group {
	enum : asset_group_t {
		sampler,
		image,
		uniform,

		num_asset_groups,
	};
	}
}