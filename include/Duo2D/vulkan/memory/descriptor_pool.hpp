#pragma once
#include <bitset>
#include <cstdint>
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorPool);

namespace d2d::vk {
    struct descriptor_pool : vulkan_ptr<VkDescriptorPool, vkDestroyDescriptorPool> {
        template<std::size_t N>
        static result<descriptor_pool> create(std::shared_ptr<logical_device> device, std::span<VkDescriptorPoolSize, N> pool_sizes, std::uint32_t max_sets, std::bitset<N> enabled_bindings) noexcept;
    };
}

#include "Duo2D/vulkan/memory/descriptor_pool.inl"