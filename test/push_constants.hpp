#pragma once
#include <vulkan/vulkan.h>
#include <sirius/arith/size.hpp>

struct draw_constants {
	acma::extent2 swap_extent;
	VkDeviceAddress position_buff_addr;
};

struct compute_constants {
	VkDeviceAddress buffer_addresses_addr;
};