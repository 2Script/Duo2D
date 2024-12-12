#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);

namespace d2d {
    struct device_memory : vulkan_ptr<VkDeviceMemory, vkFreeMemory> {
        static result<device_memory> create(logical_device& device, physical_device& phys_device, buffer& associated_buffer, VkMemoryPropertyFlags properties) noexcept;
    };
}
