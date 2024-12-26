#pragma once
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <type_traits>

namespace d2d {
    template<typename T>
    struct renderable_traits;

    template<typename T>
    struct renderable : renderable_traits<T> { using traits_type = renderable_traits<T>; };
}


namespace d2d::impl {
    template<typename T>
    concept RenderableType = requires {
        typename T::traits_type;
        requires ShaderType<typename T::shader_type>;
        requires std::is_standard_layout_v<typename T::instance_type> || !T::instanced;
        requires (requires(T t) {{t.instance()} noexcept -> std::same_as<typename T::instance_type>;}) || !T::instanced;
        requires (requires(T t) {{t.vertices()} noexcept;}) || T::instanced;
    };

    template<typename T>
    constexpr bool has_indices_v = RenderableType<T> && requires {
        requires (requires(T t) {{t.indices()} noexcept;}) || (requires{ {T::indices()} noexcept;});
        {T::index_count} -> std::same_as<const std::size_t&>;
        requires T::index_count > 0;
        requires std::is_same_v<typename T::index_type, std::uint16_t> || std::is_same_v<typename T::index_type, std::uint32_t>;
    };

    template<typename T>
    constexpr bool has_vertices_v = RenderableType<T> && requires {
        requires (requires(T t) {{t.vertices()} noexcept;}) || (requires{ {T::vertices()} noexcept;});
        {T::vertex_count} -> std::same_as<const std::size_t&>;
        requires T::vertex_count > 0;
        typename T::vertex_type;
    };

    template<typename T>
    constexpr bool has_uniform_v = RenderableType<T> && requires { typename T::uniform_type; };

    template<typename T>
    constexpr bool has_push_constants_v = RenderableType<T> && requires {
        {T::push_constants()} noexcept; 
        {T::push_constant_ranges()} noexcept;
        requires std::tuple_size_v<typename T::push_constant_types> == T::push_constant_ranges().size();
    };

    template<typename T>
    constexpr bool has_attributes_v = RenderableType<T> && requires (T t) {
        {t.attributes()} noexcept; 
        std::get<0>(t.attributes());
        std::tuple_size_v<typename T::attribute_types>;
    };

    template<typename T>
    constexpr bool has_storage_v = RenderableType<T> && false; //TODO
}



namespace d2d::impl { 
    template<typename T> struct extract_attribute_size;
    template<typename... Ts> 
    struct extract_attribute_size<std::tuple<attribute<Ts>&...>> : 
        std::integral_constant<std::size_t, (sizeof(Ts) + ...)> {};

    template<typename T>
    struct attributes_span_type { using type = std::tuple<>; };

    template<typename T> requires has_attributes_v<T>
    struct attributes_span_type<T> { using type = std::tuple<std::span<std::byte>>; };

    template<typename... Ts>
    using attributes_tuple = decltype(std::tuple_cat(
        std::declval<typename attributes_span_type<Ts>::type>()...
    )); 
}

namespace d2d {
    template<typename... Ts>
    struct make_attribute_types { using type = std::tuple<attribute<Ts>&...>; };

    template<typename... Ts>
    using make_attribute_types_t = typename make_attribute_types<Ts...>::type;
}