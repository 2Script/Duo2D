#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkCommandPool);


namespace d2d::vk {
    struct command_pool : vulkan_ptr<VkCommandPool, vkDestroyCommandPool> {
        static result<command_pool> create(command_family_t family, std::shared_ptr<logical_device> logi_deivce, std::weak_ptr<physical_device> phys_device) noexcept;
    };
}
