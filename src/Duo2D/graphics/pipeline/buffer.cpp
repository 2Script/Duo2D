#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/error.hpp"


namespace d2d {
    result<buffer> buffer::create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept {
        buffer ret{};
        ret.dependent_handle = device;

        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        __D2D_VULKAN_VERIFY(vkCreateBuffer(device, &buffer_info, nullptr, &ret.handle));
        return ret;
    }
}