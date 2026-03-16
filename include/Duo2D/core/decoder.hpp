#pragma once
#include <array>
#include <vector>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/memory/unique_ptr.hpp>

#include <llfio.hpp>
#include <ktx.h>
#include <harfbuzz/hb.h>

#include "Duo2D/arith/point.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/core/texture.hpp"


namespace llfio = LLFIO_V2_NAMESPACE;

namespace d2d {
	namespace decoder {
		namespace font_texture {
        	constexpr sl::size_t length_pixels = 32;
        	constexpr sl::size_t channels = 4;
        	constexpr sl::size_t size_bytes = length_pixels * length_pixels * channels;
        	constexpr double padding_em = 0.0625;
        	constexpr double distance_range = 0.125;
        	constexpr sl::size_t glyph_scale = length_pixels;
		}
	}


	namespace decoder {
		result<llfio::mapped_file_handle> open_file(llfio::path_view path) noexcept;
	}

	namespace decoder { 
		result<texture>
		decode_texture(llfio::mapped_file_handle const& handle, texture_usage usage) noexcept;
 
		result<std::vector<std::array<std::byte, font_texture::size_bytes>>>
		decode_font(llfio::mapped_file_handle const& handle) noexcept;
		
	};
}

namespace d2d::impl {
    struct glyph_context {
        pt2d pos;
        double scale;
    };

	[[gnu::nonnull]] constexpr void move_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void line_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void quad_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float control_x, float control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
    [[gnu::nonnull]] constexpr void cubic_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float first_control_x, float first_control_y, float second_control_x, float second_control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
}