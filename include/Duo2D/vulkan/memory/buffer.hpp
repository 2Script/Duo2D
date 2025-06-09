#pragma once
#include <compare>
#include <cstring>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);

namespace d2d {
    struct buffer : public vulkan_ptr<VkBuffer, vkDestroyBuffer> {
        static result<buffer> create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept;

        result<buffer> clone(logical_device& device) const noexcept;
        result<buffer> clone(logical_device& device, physical_device&) const noexcept;

    public:
        constexpr std::size_t size() const noexcept { return bytes; }
        constexpr std::size_t size_bytes() const noexcept { return bytes; }
        constexpr bool empty() const noexcept { return bytes == 0; } 

    private:
        std::size_t bytes = 0;
        VkBufferUsageFlags flags;
    };
}
