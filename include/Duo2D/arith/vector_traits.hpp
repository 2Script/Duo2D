#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vulkan/vulkan.h>

namespace d2d::impl {
    enum transform_flags {
        scale = 0b001, rotate = 0b010, translate = 0b100
    };

    template<std::size_t Dims, typename UnitTy, bool HoldsSize, std::uint8_t TransformFlags>
    struct vector_traits {
        using vk_type = std::conditional_t<!HoldsSize,
            std::conditional_t<Dims == 2, VkOffset2D, VkOffset3D>, 
            std::conditional_t<Dims == 2, VkExtent2D, VkExtent3D>
        >;

        using vk_component_type = std::conditional_t<!HoldsSize,
            decltype(std::declval<VkOffset2D>().x), 
            decltype(std::declval<VkExtent2D>().height)
        >;

        constexpr static bool scalable     = !(TransformFlags & (rotate | translate));
        constexpr static bool rotatable    = !(TransformFlags & translate);
        constexpr static bool translatable = true;
    };

    template<std::size_t Dims, typename UnitTy>
    struct vertex_traits {
        constexpr static VkFormat format = VK_FORMAT_UNDEFINED;
    };

    template<std::size_t Dims>
    concept Cartesian = Dims == 2 || Dims == 3;

    template<std::size_t Dims>
    concept Graphical = Cartesian<Dims> || Dims == 4;

    template<std::size_t Dims, typename T, bool HoldsSize, std::uint8_t TransformFlags>
    concept VkCompatibleType = Cartesian<Dims> && std::is_convertible_v<T, typename vector_traits<Dims, T, HoldsSize, TransformFlags>::vk_component_type>; 
}


namespace d2d::impl {
    template<> struct vertex_traits<1, float> { constexpr static VkFormat format = VK_FORMAT_R32_SFLOAT; };
    template<> struct vertex_traits<2, float> { constexpr static VkFormat format = VK_FORMAT_R32G32_SFLOAT; };
    template<> struct vertex_traits<3, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32_SFLOAT; };
    template<> struct vertex_traits<4, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT; };
}