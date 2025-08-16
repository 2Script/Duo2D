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

namespace d2d::vk {
    struct buffer : public vulkan_ptr<VkBuffer, vkDestroyBuffer> {
        static result<buffer> create(std::shared_ptr<logical_device> device, std::size_t size, VkBufferUsageFlags usage) noexcept;
    private:
        static result<buffer> create(std::shared_ptr<logical_device> device, std::size_t size, VkBufferUsageFlags usage, std::size_t mem_offset) noexcept;

    public:
        result<buffer> clone(std::shared_ptr<logical_device> device) const noexcept;
        result<buffer> clone(std::shared_ptr<logical_device> device, std::weak_ptr<physical_device>) const noexcept;

    public:
        constexpr std::size_t size() const noexcept { return bytes; }
        constexpr std::size_t size_bytes() const noexcept { return bytes; }
        constexpr bool empty() const noexcept { return bytes == 0; }
        constexpr VkBufferUsageFlags usage_flags() const noexcept { return flags; }
        
        constexpr std::size_t memory_offset() const noexcept { return offset; }

    private:
        std::size_t bytes = 0;
        std::size_t offset = 0;
        VkBufferUsageFlags flags;
    public:    
        template<std::size_t N>
        friend struct device_memory;
    };
}
