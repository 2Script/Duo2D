#pragma once

#include <vulkan/vulkan.h>


namespace d2d::vk {
	union descriptor_info {
		VkDescriptorImageInfo image;
		VkDescriptorBufferInfo buffer;
	};
}