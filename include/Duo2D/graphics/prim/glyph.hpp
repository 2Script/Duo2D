#pragma once
#include <cstddef>
#include <cstdint>
#include <tuple>

#include <vulkan/vulkan.h>

#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include "Duo2D/shaders/glyph.hpp"


namespace d2d {
    using glyph_idx_t = std::uint_least16_t;

    struct glyph;
    template<> struct renderable_traits<glyph> {
        using shader_type = glyph;
        constexpr static auto vert_shader_data = std::to_array(shaders::glyph::vert);
        constexpr static auto frag_shader_data = std::to_array(shaders::glyph::frag);
        struct uniform_type {
            extent2 swap_chain_extent;
        };
        using push_constant_types = std::tuple<extent2&>;
        using attribute_types = vk::make_attribute_types_t<glyph_idx_t, std::uint_least16_t, pt2<float>, true_color>;
        using asset_type = font;
        using index_type = std::uint16_t;

        constexpr static std::size_t index_count = 6;
        constexpr static std::size_t max_texture_count = 1;

        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
}


namespace d2d {
    struct glyph : renderable<glyph> {
        vk::attribute<glyph_idx_t> glyph_idx;
        vk::attribute<std::uint_least16_t> size;
        vk::attribute<pt2<float>> pos;
        vk::attribute<true_color> color;

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 1, 3}; }
        constexpr attribute_types attributes() noexcept { return std::tie(glyph_idx, size, pos, color); }
    
    public:
        template<typename... Ts, typename UniformT, std::size_t N>
        constexpr static void on_swap_chain_update(basic_window<Ts...> const& win, std::span<UniformT, N> uniform_map) noexcept {
            for(std::size_t i = 0; i < N; ++i) std::memcpy(&uniform_map[i], &win.swap_chain().extent(), sizeof(UniformT));
        }
    };
}
