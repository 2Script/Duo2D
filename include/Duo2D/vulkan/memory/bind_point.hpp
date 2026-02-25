#pragma once
#include <streamline/metaprogramming/underlying_type.hpp>

#include <vulkan/vulkan.h>

namespace d2d::vk {
	using bind_point_t = sl::underlying_type_t<VkPipelineBindPoint>;

	namespace bind_point {
	enum : bind_point_t {
		graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
		compute = VK_PIPELINE_BIND_POINT_COMPUTE,
	};
	}
}