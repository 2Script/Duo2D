#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    result<buffer> buffer::create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept {
        buffer ret{};
        ret.dependent_handle = device;
        ret.buff_type = buffer_type::generic;
        ret.flags = usage;
        ret.buff_size.bytes = size;

        VkBufferCreateInfo generic_buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        __D2D_VULKAN_VERIFY(vkCreateBuffer(device, &generic_buffer_info, nullptr, &ret.handle.generic));
        return ret;
    }
}

namespace d2d {
    result<buffer> buffer::create(logical_device& device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) noexcept {
        buffer ret{};
        ret.dependent_handle = device;
        //ret.size_bytes = width * height * 4;
        ret.buff_type = buffer_type::image;
        ret.flags = usage;
        ret.buff_size.extent = {width, height};

        ret.image_format = format;
        ret.image_tiling = tiling;

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

        __D2D_VULKAN_VERIFY(vkCreateImage(device, &image_buffer_info, nullptr, &ret.handle.image));
        return ret;
    }
}


namespace d2d {
    result<buffer> buffer::clone(logical_device& device) noexcept {
        switch(buff_type) {
        case buffer_type::generic:
            return buffer::create(device, buff_size.bytes, flags);
        case buffer_type::image:
            return buffer::create(device, buff_size.extent.width(), buff_size.extent.height(), image_format, image_tiling, flags);
        }
    }
}