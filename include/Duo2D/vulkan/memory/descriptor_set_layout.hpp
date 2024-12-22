#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorSetLayout);

namespace d2d {
    struct descriptor_set_layout : vulkan_ptr<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout> {
        static result<descriptor_set_layout> create(logical_device& device) noexcept;
    };
}