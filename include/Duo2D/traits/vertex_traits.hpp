#pragma once
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>


namespace d2d::impl {
    template<std::size_t Dims, typename UnitTy>
    struct vertex_traits {
        constexpr static VkFormat format = VK_FORMAT_UNDEFINED;
    };
}


namespace d2d::impl {
    template<> struct vertex_traits<1, float> { constexpr static VkFormat format = VK_FORMAT_R32_SFLOAT; };
    template<> struct vertex_traits<2, float> { constexpr static VkFormat format = VK_FORMAT_R32G32_SFLOAT; };
    template<> struct vertex_traits<3, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32_SFLOAT; };
    template<> struct vertex_traits<4, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT; };

    template<> struct vertex_traits<1, std::uint8_t> { constexpr static VkFormat format = VK_FORMAT_R8_UINT; };
    template<> struct vertex_traits<2, std::uint8_t> { constexpr static VkFormat format = VK_FORMAT_R8G8_UINT; };
    template<> struct vertex_traits<3, std::uint8_t> { constexpr static VkFormat format = VK_FORMAT_R8G8B8_UINT; };
    template<> struct vertex_traits<4, std::uint8_t> { constexpr static VkFormat format = VK_FORMAT_R8G8B8A8_UINT; };

    template<> struct vertex_traits<1, std::uint16_t> { constexpr static VkFormat format = VK_FORMAT_R16_UINT; };
    template<> struct vertex_traits<2, std::uint16_t> { constexpr static VkFormat format = VK_FORMAT_R16G16_UINT; };
    template<> struct vertex_traits<3, std::uint16_t> { constexpr static VkFormat format = VK_FORMAT_R16G16B16_UINT; };
    template<> struct vertex_traits<4, std::uint16_t> { constexpr static VkFormat format = VK_FORMAT_R16G16B16A16_UINT; };
}