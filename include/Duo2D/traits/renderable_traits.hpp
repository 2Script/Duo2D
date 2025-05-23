#pragma once
#include "Duo2D/traits/shader_traits.hpp"
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<typename T>
    struct renderable_traits;
}

namespace d2d {
    template<typename T>
    struct renderable_constraints {
        constexpr static bool has_indices = requires {
            requires (requires(T t) {{t.indices()} noexcept;}) || (requires{ {T::indices()} noexcept;});
            requires std::is_same_v<typename T::index_type, std::uint16_t> || std::is_same_v<typename T::index_type, std::uint32_t>;
        };

        constexpr static bool has_vertices = requires {
            requires (requires(T t) {{t.vertices()} noexcept;}) || (requires{ {T::vertices()} noexcept;});
            typename T::vertex_type;
        };

        constexpr static bool has_fixed_indices = has_indices && requires {
            {T::index_count} -> std::same_as<const std::size_t&>;
            requires T::index_count > 0;
        };

        constexpr static bool has_fixed_vertices = has_vertices && requires {
            {T::vertex_count} -> std::same_as<const std::size_t&>;
            requires T::vertex_count > 0;
        };

        constexpr static bool has_uniform = requires { typename T::uniform_type; };

        constexpr static bool has_push_constants = requires {
            {T::push_constants()} noexcept; 
            {T::push_constant_ranges()} noexcept;
            requires std::tuple_size_v<typename T::push_constant_types> == T::push_constant_ranges().size();
        };

        constexpr static bool has_attributes = requires (T t) {
            {t.attributes()} noexcept; 
            std::get<0>(t.attributes());
            std::tuple_size_v<typename T::attribute_types>;
        };

        constexpr static bool has_storage = false; //TODO
    };
}


namespace d2d::impl {
    template<typename T>
    concept renderable_like = requires {
        typename T::traits_type;
        requires shader_like<typename T::shader_type>;
        requires std::is_standard_layout_v<typename T::instance_type> || !T::instanced;
        requires (requires(T t) {{t.instance()} noexcept -> std::same_as<typename T::instance_type>;}) || !T::instanced;
        requires (requires(T t) {{t.vertices()} noexcept;}) || T::instanced;
    };
}

