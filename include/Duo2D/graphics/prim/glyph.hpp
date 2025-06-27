#pragma once
#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include "Duo2D/shaders/glyph.hpp"
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct glyph;
    template<> struct shader_traits<glyph> {
        using shader_type = glyph;
        using shader_data_type = shaders::glyph;
        struct uniform_type {
            extent2 swap_chain_extent;
        };
        using push_constant_types = std::tuple<extent2&>;
        using attribute_types = vk::make_attribute_types_t<true_color>;
        constexpr static std::size_t max_texture_count = 1;
        using texture_type = font;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
    template<> struct renderable_traits<glyph> : shader_traits<glyph> {
        using index_type = std::uint16_t;
        struct instance_type {
            std::uint16_t glyph_idx;
            pt2<float> pos;
            size2<float> size;
        };
        constexpr static std::size_t index_count = 6;
    };
}


namespace d2d {
    struct glyph : renderable<glyph> {
        std::uint16_t glyph_idx;
        pt2<float> pos;
        size2<float> size;
        vk::attribute<true_color> color = {};

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 1, 3}; }
        constexpr instance_type instance() const noexcept { return {glyph_idx, pos, size}; }
        constexpr attribute_types attributes() noexcept { return std::tie(color); }
    };
}
