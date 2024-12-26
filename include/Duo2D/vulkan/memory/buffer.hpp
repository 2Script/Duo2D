#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);

namespace d2d {
    struct buffer : vulkan_ptr<VkBuffer, vkDestroyBuffer> {
        static result<buffer> create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept;

    public:
        constexpr std::size_t size() const noexcept { return size_bytes; } 
        constexpr bool empty() const noexcept { return size_bytes == 0; } 
        constexpr VkBufferUsageFlags usage_flags() const noexcept { return flags; } 
    private:
        std::size_t size_bytes = 0;
        VkBufferUsageFlags flags;
    };
}
