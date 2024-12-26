#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace d2d::impl {
    enum transform_flags {
        scale = 0b001, rotate = 0b010, translate = 0b100
    };

    enum class vec_data_type {
        none, point, size
    };

    template<std::size_t Dims, typename UnitTy, vec_data_type HoldsData, std::uint8_t TransformFlags>
    struct vector_traits {
        using vk_type = std::conditional_t<HoldsData != vec_data_type::size,
            std::conditional_t<Dims == 2, VkOffset2D, VkOffset3D>, 
            std::conditional_t<Dims == 2, VkExtent2D, VkExtent3D>
        >;

        using vk_component_type = std::conditional_t<HoldsData != vec_data_type::size,
            decltype(std::declval<VkOffset2D>().x), 
            decltype(std::declval<VkExtent2D>().height)
        >;

        constexpr static bool scalable     = !(TransformFlags & (rotate | translate));
        constexpr static bool rotatable    = !(TransformFlags & (rotate | translate)); //TODO distinguish between rotating same axis and different (not allowed) axis
        constexpr static bool translatable = true;
    };
    
    template<std::size_t Dims>
    concept Cartesian = Dims == 2 || Dims == 3;

    template<std::size_t Dims>
    concept Graphical = Cartesian<Dims> || Dims == 4;

    template<std::size_t Dims, typename T, vec_data_type HoldsData, std::uint8_t TransformFlags>
    concept VkCompatibleType = Cartesian<Dims> && std::is_convertible_v<T, typename vector_traits<Dims, T, HoldsData, TransformFlags>::vk_component_type>; 
}