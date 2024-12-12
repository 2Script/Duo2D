#pragma once
#include "Duo2D/traits/shader_traits.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace d2d {
    template<typename T>
    struct renderable_traits;
}


namespace d2d::impl {
    template<typename T>
    concept RenderableType = requires {
        typename T::traits_type;
        requires ShaderType<typename T::shader_type>;
        requires std::is_same_v<typename T::index_type, std::uint16_t> || std::is_same_v<typename T::index_type, std::uint32_t>;
        typename T::vertex_type;
        typename T::instance_type;
        requires (requires(T t) {{t.instance()} noexcept -> std::same_as<typename T::instance_type>;}) || !T::instanced;
        requires (requires(T t) {{t.vertices()} noexcept;}) || T::instanced;
        {T::vertex_count} -> std::same_as<const std::size_t&>;
        {T::index_count}  -> std::same_as<const std::size_t&>;
    };

    template<typename T>
    concept IndexRenderableType = RenderableType<T> && ((requires(T t) {{t.indices()} noexcept;}) || (requires{ {T::indices()} noexcept;})) && T::index_count > 0;

    template<typename T>
    concept VertexRenderableType = RenderableType<T> && ((((requires(T t) {{t.vertices()} noexcept;}) || (requires{ {T::vertices()} noexcept;})) && T::vertex_count > 0 && !std::is_same_v<typename T::vertex_type, void>));

    template<typename T>
    concept UniformRenderableType = RenderableType<T> && !std::is_same_v<typename T::uniform_type, void>;

}

#define __D2D_DECLARE_RENDERABLE_TRAITS(renderable_traits_t) \
__D2D_DECLARE_SHADER_TRAITS(renderable_traits_t); \
using shader_type = typename renderable_traits_t::shader_type; \
using index_type = typename renderable_traits_t::index_type; \
using vertex_type = typename renderable_traits_t::vertex_type; \
using instance_type = typename renderable_traits_t::instance_type; \
constexpr static std::size_t vertex_count = renderable_traits_t::vertex_count; \
constexpr static std::size_t index_count  = renderable_traits_t::index_count;