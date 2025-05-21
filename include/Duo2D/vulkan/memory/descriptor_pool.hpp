#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorPool);

namespace d2d {
    template<std::uint32_t FramesInFlight>
    struct descriptor_pool : vulkan_ptr<VkDescriptorPool, vkDestroyDescriptorPool> {
        static result<descriptor_pool> create(logical_device& device, std::uint32_t descriptor_count) noexcept;
    };
}

#include "Duo2D/vulkan/memory/descriptor_pool.inl"