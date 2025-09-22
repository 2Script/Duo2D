#pragma once
#include <concepts>
#include <array>
#include <string_view>

#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/graphics/core/texture.hpp"


namespace d2d::impl {
    template<typename T, typename TextureT>
    concept has_asset_type = requires {
        {renderable_traits<T>::max_texture_count} -> std::same_as<const std::size_t&>;
        requires renderable_traits<T>::max_texture_count > 0;
        requires std::is_same_v<typename renderable_traits<T>::asset_type, TextureT>;
    };

    template<typename T>
    struct renderable_textures {};

    template<has_asset_type<texture> T>
    struct renderable_textures<T> {
        std::array<texture_view, renderable_traits<T>::max_texture_count> _textures{};

        constexpr std::array<texture_view, renderable_traits<T>::max_texture_count>      & texture_names()       noexcept { return _textures; }
        constexpr std::array<texture_view, renderable_traits<T>::max_texture_count> const& texture_names() const noexcept { return _textures; }
    };
    template<has_asset_type<font> T>
    struct renderable_textures<T> {
        std::array<font_view, renderable_traits<T>::max_texture_count> _fonts{};

        constexpr std::array<font_view, renderable_traits<T>::max_texture_count>      & texture_names()       noexcept { return _fonts; }
        constexpr std::array<font_view, renderable_traits<T>::max_texture_count> const& texture_names() const noexcept { return _fonts; }
    };
}