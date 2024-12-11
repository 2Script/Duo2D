#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);

namespace d2d {
    struct buffer : pipeline_obj<VkBuffer, vkDestroyBuffer> {
        static result<buffer> create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept;

    public:
        std::size_t size() const noexcept { return size_bytes; } 
    private:
        std::size_t size_bytes;
    };
}
