#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);

namespace d2d {
    struct device_memory : pipeline_obj<VkDeviceMemory, vkFreeMemory> {
        static result<device_memory> create(logical_device& device, physical_device& phys_device, buffer& associated_buffer, VkMemoryPropertyFlags properties) noexcept;
    };
}
