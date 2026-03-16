#pragma once
#include <streamline/numeric/int.hpp>


namespace d2d::vk {
	using descriptor_heap_t = sl::uint8_t;
}

namespace d2d::vk {
	namespace descriptor_heap {
	enum : descriptor_heap_t {
		sampler,
		resource,

		num_descriptor_heaps
	};
	}
}