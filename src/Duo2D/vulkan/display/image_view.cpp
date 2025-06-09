#include "Duo2D/vulkan/display/image_view.hpp"

namespace d2d {
    result<image_view> image_view::create(logical_device& device, VkImage img, VkFormat format) noexcept {
        image_view ret{};
        ret.dependent_handle = device;
        VkImageViewCreateInfo image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = img,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        __D2D_VULKAN_VERIFY(vkCreateImageView(device, &image_view_create_info, nullptr, &ret));
        return ret;
    }
}