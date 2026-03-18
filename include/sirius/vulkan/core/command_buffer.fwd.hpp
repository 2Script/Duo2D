#pragma once

#include <vulkan/vulkan.h>

#include "sirius/vulkan/core/command_pool.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE_AUX(VkCommandBuffer, command_pool);

namespace acma::vk {
	struct command_buffer;
}