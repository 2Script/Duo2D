#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>


namespace d2d {
    using memory_policy_t = std::uint8_t;

	namespace memory_policy {
    enum : memory_policy_t {
        gpu_local, 
        cpu_local, 
        shared,
        push_constant,

        num_memory_policies,
		num_allocation_backed_memory_policies = num_memory_policies - 1,

    };
	}
}

namespace d2d::impl {
	template<memory_policy_t MemoryPolicy>
	constexpr VkMemoryPropertyFlags flags_for = 0;

	template<> inline 
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::gpu_local> = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	template<> inline 
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::cpu_local> = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	template<> inline 
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::shared> = flags_for<memory_policy::gpu_local> | flags_for<memory_policy::cpu_local>;
}