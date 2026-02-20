#pragma once

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/core/command_pool.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE_AUX(VkCommandBuffer, command_pool);

namespace d2d::vk {
	template<sl::size_t N>
	struct command_buffer;
}