#pragma once
#include <cstdint>

#include <vulkan/vulkan.h>


namespace d2d {
    using memory_policy_t = std::uint8_t;

	namespace memory_policy {
    enum : memory_policy_t {
        gpu_local, 
        cpu_local_cpu_write, 
		cpu_local_gpu_write,
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
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::cpu_local_cpu_write> = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	template<> inline 
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::cpu_local_gpu_write> = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	template<> inline 
	constexpr VkMemoryPropertyFlags flags_for<memory_policy::shared> = flags_for<memory_policy::gpu_local> | flags_for<memory_policy::cpu_local_cpu_write>;
}

namespace d2d {
	namespace memory_policy {
		constexpr bool is_cpu_visible(memory_policy_t mp) noexcept {
			return mp != memory_policy::gpu_local;
		}

		constexpr bool is_cpu_writable(memory_policy_t mp) noexcept {
			return is_cpu_visible(mp) && mp != memory_policy::cpu_local_gpu_write;
		}
	}
}