#pragma once
#include <cstddef>

#include <harfbuzz/hb.h>
#include "Duo2D/arith/point.hpp"
#include "Duo2D/vulkan/memory/texture_map_base.hpp"


namespace d2d::vk {
    //using path_view = LLFIO_V2_NAMESPACE::path_view;

    class texture_map : public texture_map_base {
    private:
        using texture_map_base::insert;
        using texture_map_base::insert_or_assign;
        using texture_map_base::emplace;
        using texture_map_base::emplace_hint;
        using texture_map_base::try_emplace;
        using texture_map_base::swap;
        using texture_map_base::extract;
        using texture_map_base::merge;
        using texture_map_base::at;
        using texture_map_base::operator[];

        template<std::size_t FramesInFlight, typename>
        friend class renderable_tuple;

    public:
        constexpr static std::size_t font_texture_length_pixels = 32;
        constexpr static std::size_t font_texture_channels = 4;
        constexpr static std::size_t font_texture_size_bytes = font_texture_length_pixels * font_texture_length_pixels * font_texture_channels;
        constexpr static double font_texture_padding_em = 0.0625;
        constexpr static double font_texture_distance_range = 0.125;
        constexpr static std::size_t font_texture_glyph_scale = font_texture_length_pixels;
    };
}

namespace d2d::vk::impl {
    struct glyph_context {
        pt2d pos;
        double scale;
    };
}

namespace d2d::vk::impl::draw_op {
    [[gnu::nonnull]] constexpr void move_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void line_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void quad_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float control_x, float control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void cubic_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float first_control_x, float first_control_y, float second_control_x, float second_control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
}

#include "Duo2D/vulkan/memory/texture_map.inl"