#pragma once
#include <compare>
#include <cstring>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);
__D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);

namespace d2d {
    enum class buffer_type : bool {
        generic, image
    };
}

namespace d2d {
    struct buffer {
        static result<buffer> create(logical_device& device, std::size_t size, VkBufferUsageFlags usage) noexcept;
        static result<buffer> create(logical_device& device, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) noexcept;
        result<buffer> clone(logical_device& device) noexcept;
        
    public:
        constexpr explicit operator VkBuffer() const noexcept { return handle.generic; }
        constexpr explicit operator VkImage() const noexcept { return handle.image; }
        constexpr void*       operator&()       noexcept { return buff_type == buffer_type::image ? static_cast<void*      >(&handle.image) : static_cast<void*      >(&handle.generic); }
        constexpr void const* operator&() const noexcept { return buff_type == buffer_type::image ? static_cast<void const*>(&handle.image) : static_cast<void const*>(&handle.generic);  }
    public:
        constexpr explicit operator bool() const noexcept { return buff_type == buffer_type::image ? (handle.image != VK_NULL_HANDLE) : (handle.generic != VK_NULL_HANDLE); }
        constexpr std::strong_ordering operator<=>(buffer const& o) const noexcept { return buff_type == buffer_type::image ? (handle.image <=> o.handle.image) : (handle.generic <=> o.handle.generic);};
        constexpr bool operator==(buffer const& o) const noexcept { return buff_type == buffer_type::image ? (handle.image == o.handle.image) : (handle.generic == o.handle.generic);};

    public:
        constexpr bool empty() const noexcept { return buff_type == buffer_type::image ? (buff_size.extent.empty()) : (buff_size.bytes == 0); } 
        constexpr buffer_type type() const noexcept { return buff_type; }
    public:
        constexpr std::size_t size() const noexcept { return buff_size.bytes; } 
        constexpr extent2 image_size() const noexcept { return buff_size.extent; } 

    private:
        union {
            VkBuffer generic = VK_NULL_HANDLE;
            VkImage image;
        } handle;
        VkDevice dependent_handle;
    private:
        buffer_type buff_type;
        VkFlags flags;
    private:
        union {
            std::size_t bytes;
            extent2 extent;
        } buff_size;
        VkFormat image_format;
        VkImageTiling image_tiling;
        //VkImageLayout image_layout;


    private:
        constexpr void destroy() const noexcept {
            switch(buff_type) {
            case buffer_type::generic:
                vkDestroyBuffer(dependent_handle, handle.generic, nullptr);
                break;
            case buffer_type::image:
                vkDestroyImage(dependent_handle, handle.image, nullptr);
                break;
            };
        }

    public:
        constexpr buffer() noexcept = default;
        ~buffer() noexcept { if(*this) destroy(); }
        constexpr buffer(buffer&& other) noexcept : 
            handle(other.handle),
            dependent_handle(other.dependent_handle), 
            buff_type(other.buff_type), 
            flags(other.flags), 
            buff_size(other.buff_size),
            image_format(other.image_format), 
            image_tiling(other.image_tiling) {

            other.handle.generic = nullptr;
        }
        constexpr buffer& operator=(buffer&& other) noexcept { 
            if(*this && *this != other) destroy();
            handle = other.handle;
            dependent_handle = other.dependent_handle;
            buff_type = other.buff_type;
            flags = other.flags;
            buff_size = other.buff_size;
            image_format = other.image_format;
            image_tiling = other.image_tiling;

            other.handle.generic = nullptr;
            return *this;
        };

        buffer(const buffer& other) = delete;
        buffer& operator=(const buffer& other) = delete;
    };
}
