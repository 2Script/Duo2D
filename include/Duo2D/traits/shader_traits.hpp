#pragma once
#include <concepts>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<typename T>
    struct shader_traits;
}

namespace d2d::impl {
    template<typename T>
    concept ShaderType = requires {
        //requires std::is_same_v<typename T::traits_type, shader_traits<T>>;
        {T::binding_descs()} noexcept;
        {T::attribute_descs()} noexcept;
        typename T::shader_data_type;
        typename T::uniform_type;
        {T::instanced} -> std::same_as<const bool&>;
        {T::cull_mode} -> std::same_as<const VkCullModeFlags&>;
        {T::front_face} -> std::same_as<const VkFrontFace&>;
    };

}

#define __D2D_DECLARE_NESTED_TRAITS(shader_traits_t) \
using shader_data_type = typename shader_traits_t::shader_data_type; \
using uniform_type = typename shader_traits_t::uniform_type; \
constexpr static bool instanced = shader_traits_t::instanced; \
constexpr static VkCullModeFlags cull_mode = shader_traits_t::cull_mode; \
constexpr static VkFrontFace front_face = shader_traits_t::front_face;

#define __D2D_DECLARE_SHADER_TRAITS(shader_traits_t) \
using traits_type = shader_traits_t; \
__D2D_DECLARE_NESTED_TRAITS(shader_traits_t);