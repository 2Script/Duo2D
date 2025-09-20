#pragma once
#include <concepts>
#include <array>
#include <string_view>
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/vulkan/display/sampled_image.hpp"

namespace d2d::impl {
    template<typename T, typename TextureT>
    concept has_texture_type = requires {
        {renderable_traits<T>::max_texture_count} -> std::same_as<const std::size_t&>;
        requires renderable_traits<T>::max_texture_count > 0;
        requires std::is_same_v<typename renderable_traits<T>::texture_type, TextureT>;
    };

    template<typename T>
    struct renderable_textures {};

    template<has_texture_type<vk::sampled_image> T>
    struct renderable_textures<T> {
        std::array<std::string_view, renderable_traits<T>::max_texture_count> _texture_paths{};

        constexpr std::array<std::string_view, renderable_traits<T>::max_texture_count> const& texture_keys() const noexcept { return _texture_paths; }
    };
    template<has_texture_type<font> T>
    struct renderable_textures<T> {
        std::array<font_view, renderable_traits<T>::max_texture_count> _fonts{};

        constexpr std::array<font_view, renderable_traits<T>::max_texture_count> const& texture_keys() const noexcept { return _fonts; }
    };
}