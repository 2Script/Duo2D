#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/core/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    result<buffer> buffer::create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept {
        buffer ret{};
        ret.dependent_handle = device;
        ret.flags = usage;
        ret.bytes = size;

        VkBufferCreateInfo generic_buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        __D2D_VULKAN_VERIFY(vkCreateBuffer(device, &generic_buffer_info, nullptr, &ret.handle));
        return ret;
    }
}

namespace d2d {
    result<buffer> buffer::clone(logical_device& device) const noexcept {
        return buffer::create(device, bytes, flags);
    }

    result<buffer> buffer::clone(logical_device& device, physical_device&) const noexcept {
        return clone(device);
    }
}