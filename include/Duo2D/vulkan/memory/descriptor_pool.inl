#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/core/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d::vk {
    template<std::size_t N>
    result<descriptor_pool> descriptor_pool::create(logical_device& device, std::span<VkDescriptorPoolSize, N> pool_sizes, std::uint32_t max_sets, std::bitset<N> enabled_bindings) noexcept {
        descriptor_pool ret{};
        ret.dependent_handle = device;
        const std::uint32_t binding_count = std::numeric_limits<unsigned long long>::digits - std::countl_zero(enabled_bindings.to_ullong());

        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = max_sets,
            .poolSizeCount = binding_count,
            .pPoolSizes = pool_sizes.data(),
        };
        
        __D2D_VULKAN_VERIFY(vkCreateDescriptorPool(device, &pool_info, nullptr, &ret.handle));
        return ret;
    }
}