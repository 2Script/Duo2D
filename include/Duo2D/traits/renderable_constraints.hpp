#pragma once
#include <tuple>
#include <type_traits>
#include <cstdint>
#include <concepts>
#include "Duo2D/traits/renderable_traits.hpp"

namespace d2d {
    template<typename T>
    struct renderable_constraints {
        //constexpr static bool instanced = renderable_traits<T>::instanced;
        
        constexpr static bool has_indices = requires {
            requires (requires(T t) {{t.indices()} noexcept;}) || (requires{ {renderable_traits<T>::indices()} noexcept;});
            requires std::is_same_v<typename renderable_traits<T>::index_type, std::uint16_t> || std::is_same_v<typename renderable_traits<T>::index_type, std::uint32_t>;
        };

        constexpr static bool has_vertices = requires {
            requires (requires(T t) {{t.vertices()} noexcept;}) || (requires{ {renderable_traits<T>::vertices()} noexcept;});
            typename renderable_traits<T>::vertex_type;
        };

        constexpr static bool has_fixed_indices = has_indices && requires {
            {renderable_traits<T>::index_count} -> std::same_as<const std::size_t&>;
            requires renderable_traits<T>::index_count > 0;
        };

        constexpr static bool has_fixed_vertices = has_vertices && requires {
            {renderable_traits<T>::vertex_count} -> std::same_as<const std::size_t&>;
            requires renderable_traits<T>::vertex_count > 0;
        };

        constexpr static bool has_instance_data = requires (T t) {
            {t.instance()} noexcept;
            typename renderable_traits<T>::instance_type;
        };

        constexpr static bool has_uniform = requires { typename renderable_traits<T>::uniform_type; };

        constexpr static bool has_textures = requires {
            {renderable_traits<T>::max_texture_count} -> std::same_as<const std::size_t&>;
            requires renderable_traits<T>::max_texture_count > 0;
        };

        constexpr static bool has_push_constants = requires {
            {T::push_constants()} noexcept; 
            {T::push_constant_ranges()} noexcept;
            requires std::tuple_size_v<typename renderable_traits<T>::push_constant_types> == T::push_constant_ranges().size();
        };

        constexpr static bool has_attributes = requires (T t) {
            {t.attributes()} noexcept; 
            std::get<0>(t.attributes());
            std::tuple_size_v<typename renderable_traits<T>::attribute_types>;
        };

        constexpr static bool has_storage = false; //TODO


        constexpr static bool instanced = has_fixed_indices || has_fixed_vertices || has_instance_data;
    };
}