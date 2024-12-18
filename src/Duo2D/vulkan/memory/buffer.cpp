#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/error.hpp"


namespace d2d {
    result<buffer> buffer::create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept {
        buffer ret{};
        ret.dependent_handle = device;
        ret.size_bytes = size;

        VkBufferCreateInfo buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        __D2D_VULKAN_VERIFY(vkCreateBuffer(device, &buffer_info, nullptr, &ret.handle));
        return ret;
    }
}