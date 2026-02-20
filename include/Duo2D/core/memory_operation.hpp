#pragma once
#include <vulkan/vulkan.h>

namespace d2d {
	using memory_operation_t = VkAccessFlags2;

	namespace memory_operation {
	enum : memory_operation_t {
		none = VK_ACCESS_2_NONE, 

		read  = VK_ACCESS_2_MEMORY_READ_BIT, 
		write = VK_ACCESS_2_MEMORY_WRITE_BIT
	};
	}
}