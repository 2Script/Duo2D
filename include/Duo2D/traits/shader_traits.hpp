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
    concept shader_like = requires {
        //requires std::is_same_v<typename T::traits_type, shader_traits<T>>;
        {T::binding_descs()} noexcept;
        {T::attribute_descs()} noexcept;
        requires std::is_same_v<typename T::shader_type, T>;
        typename T::shader_data_type;
        {renderable_constraints<T>::instanced} -> std::same_as<const bool&>;
        {T::cull_mode} -> std::same_as<const VkCullModeFlags&>;
        {T::front_face} -> std::same_as<const VkFrontFace&>;
    };

}