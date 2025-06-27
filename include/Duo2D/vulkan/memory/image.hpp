#pragma once
#include <compare>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);

namespace d2d::vk {
    struct image : public vulkan_ptr<VkImage, vkDestroyImage> {
        static result<image> create(logical_device& device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, std::uint32_t array_count = 1) noexcept;
    protected:
        static result<image> create(logical_device& device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, std::uint32_t array_count, std::size_t mem_offset) noexcept;
    
    public:
        result<image> clone(logical_device& device) const noexcept;
        
    public:
        constexpr extent2 size() const noexcept { return extent; } 
        constexpr std::size_t size_bytes() const noexcept { return bytes; } 
        constexpr bool empty() const noexcept { return bytes == 0 || extent.empty(); }
        constexpr std::size_t memory_offset() const noexcept { return offset; }

        constexpr VkFormat format() const noexcept { return image_format; }
        constexpr VkImageLayout layout() const noexcept { return image_layout; }
        constexpr VkImageLayout& layout() noexcept { return image_layout; }

    protected:
        extent2 extent = {};
        std::size_t bytes = 0;
        std::size_t offset = 0;
        VkFormat image_format;
        VkImageTiling image_tiling;
        VkImageLayout image_layout;
        VkImageUsageFlags flags;
        std::uint32_t image_count;
    public:    
        template<std::size_t N>
        friend struct device_memory;
    };
}
