#pragma once
#include <concepts>
#include <type_traits>
#include <vulkan/vulkan.h>
#include "Duo2D/traits/renderable_properties.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"

namespace d2d::impl {
    template<typename T, typename R = std::remove_cvref_t<T>>
    concept directly_renderable = requires {
        //You must have static functions that specify binding and attribute descriptions
        {renderable_properties<T>::binding_descs()} noexcept;
        {renderable_properties<T>::attribute_descs()} noexcept;
        //You must have a shader data type (i.e. a type that contains the vertex and fragment shader code bytes)
        {renderable_properties<T>::vert_shader_data.data()} -> std::same_as<const unsigned char*>;
        {renderable_properties<T>::frag_shader_data.data()} -> std::same_as<const unsigned char*>;
        {renderable_properties<T>::vert_shader_data.size()} -> std::same_as<std::size_t>;
        {renderable_properties<T>::frag_shader_data.size()} -> std::same_as<std::size_t>;
        //You must specify the cull mode and which face is the front
        {renderable_properties<T>::cull_mode} -> std::same_as<const VkCullModeFlags&>;
        {renderable_properties<T>::front_face} -> std::same_as<const VkFrontFace&>;
        //If you have instance data (i.e. instance() is a valid member function), you must have a member type `instance_type` and it must have a standard layout
        requires std::is_standard_layout_v<typename renderable_properties<R>::instance_type> || !renderable_constraints<R>::has_instance_data;
        //If you have instance data, you must have fixed vertices or fixed indices. If you don't have instance data, then we don't care.
        requires renderable_constraints<R>::has_fixed_vertices || renderable_constraints<R>::has_fixed_indices || !renderable_constraints<R>::has_instance_data; 
        //You must have vertices, unless you have fixed indices
        requires renderable_constraints<R>::has_vertices || renderable_constraints<R>::has_fixed_indices;
    };
    template<typename T>
    concept directly_renderable_file = directly_renderable<T> && (requires(T t) {{t.unload()} noexcept;});
}