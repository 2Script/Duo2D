#pragma once
#include <streamline/numeric/int.hpp>


namespace acma::vk {
	using descriptor_heap_t = sl::uint8_t;
}

namespace acma::vk {
	namespace descriptor_heap {
	enum : descriptor_heap_t {
		sampler,
		resource,

		num_descriptor_heaps
	};
	}
}