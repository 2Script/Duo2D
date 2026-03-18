#pragma once
#include <vulkan/vulkan.h>

#include "sirius/core/buffer_config_table.hpp"

namespace acma::vk {
    template<
		sl::size_t FramesInFlight, 
		typename IndexSeq,
		coupling_policy_t BufferPolicy,
		memory_policy_t MemoryPolicy,
		sl::size_t N,
		buffer_config_table<N> BufferConfigs,
		typename RenderProcessT
	>
    class device_allocation;
}
