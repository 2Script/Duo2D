#include "Duo2D/vulkan/display/image_view.hpp"

namespace d2d::vk {
    result<image_view> image_view::create(std::shared_ptr<logical_device> device, VkImage img, VkFormat format, std::uint32_t image_count, VkImageAspectFlags aspect_mask) noexcept {
        image_view ret{};
        ret.dependent_handle = device;
        VkImageViewCreateInfo image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = img,
            .viewType = image_count > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = aspect_mask,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = image_count,
            },
        };
        __D2D_VULKAN_VERIFY(vkCreateImageView(*device, &image_view_create_info, nullptr, &ret));
        return ret;
    }
}