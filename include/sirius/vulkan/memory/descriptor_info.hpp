#pragma once

#include <vulkan/vulkan.h>


namespace acma::vk {
	union descriptor_info {
		VkDescriptorImageInfo image;
		VkDescriptorBufferInfo buffer;
	};
}