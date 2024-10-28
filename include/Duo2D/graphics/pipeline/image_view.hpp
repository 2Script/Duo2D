#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkImageView);

namespace d2d {
    struct image_view : pipeline_obj<VkImageView, vkDestroyImageView> {
        static result<image_view> create(logical_device& device, VkImage& img, VkFormat format) noexcept;
    };
}
