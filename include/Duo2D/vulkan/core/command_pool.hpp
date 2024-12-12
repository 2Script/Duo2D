#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkCommandPool);


namespace d2d {
    struct command_pool : vulkan_ptr<VkCommandPool, vkDestroyCommandPool> {
        static result<command_pool> create(logical_device& logi_deivce, physical_device& phys_device) noexcept;
    };
}
