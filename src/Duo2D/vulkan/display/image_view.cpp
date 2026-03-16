#include "Duo2D/vulkan/display/image_view.hpp"

namespace d2d::vk {
    result<image_view> image_view::create(std::shared_ptr<logical_device> device, image const& img, VkImageAspectFlags aspect_mask) noexcept {
        VkImageViewCreateInfo image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = img,
            .viewType = img.layer_count() > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
            .format = img.format_id(),
            .components{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange{
                .aspectMask = aspect_mask,
                .baseMipLevel = 0,
                .levelCount = img.mip_level_count(),
                .baseArrayLayer = 0,
                .layerCount = img.layer_count(),
            },
        };
        
        return create(device, image_view_create_info);
    }
}

namespace d2d::vk {
    result<image_view> image_view::create(std::shared_ptr<logical_device> device, VkImageViewCreateInfo create_info) noexcept {
        image_view ret{};
        ret.dependent_handle = device;
        __D2D_VULKAN_VERIFY(vkCreateImageView(*device, &create_info, nullptr, &ret));
        return ret;
    }
}