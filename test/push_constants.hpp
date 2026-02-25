#pragma once
#include <vulkan/vulkan.h>
#include <Duo2D/arith/size.hpp>

struct draw_constants {
	d2d::extent2 swap_extent;
	VkDeviceAddress position_buff_addr;
};

struct compute_constants {
	VkDeviceAddress buffer_addresses_addr;
};