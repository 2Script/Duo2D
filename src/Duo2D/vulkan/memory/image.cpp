#include "Duo2D/vulkan/memory/image.hpp"

namespace d2d {
    result<image> image::create(logical_device& device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) noexcept {
        image ret{};
        ret.dependent_handle = device;
        //TODO: correct index (can't just use format)
        ret.bytes = width * height * (pixel_formats[format].total_size / 8);
        ret.extent = {width, height};
        ret.flags = usage;

        ret.image_format = format;
        ret.image_tiling = tiling;
        ret.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageCreateInfo image_buffer_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        __D2D_VULKAN_VERIFY(vkCreateImage(device, &image_buffer_info, nullptr, &ret.handle));
        return ret;
    }
}


namespace d2d {
    result<image> image::clone(logical_device& device) const noexcept {
        return image::create(device, extent.width(), extent.height(), image_format, image_tiling, flags);
    }
}