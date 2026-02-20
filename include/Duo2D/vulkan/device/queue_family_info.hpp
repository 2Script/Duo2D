#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>


namespace d2d::vk {
	struct queue_family_info {
		bool supports_present : 1;
		std::uint32_t index : 31;
	};
}
