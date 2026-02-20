#pragma once
#include <vulkan/vulkan.h>
#include <Duo2D/arith/size.hpp>

struct push_constants {
	d2d::extent2 swap_extent;
	VkDeviceAddress position_buff_addr;
};