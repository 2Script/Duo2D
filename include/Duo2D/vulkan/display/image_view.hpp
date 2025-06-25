#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkImageView);

namespace d2d {
    struct image_view : vulkan_ptr<VkImageView, vkDestroyImageView> {
        static result<image_view> create(logical_device& device, VkImage img, VkFormat format, std::uint32_t image_count = 1) noexcept;
    };
}
