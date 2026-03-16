#include "Duo2D/core/decoder.hpp"

#include <harfbuzz/hb.h>
#include <ktx.h>
#include <ktxvulkan.h>
#include <msdfgen.h>
#include <msdfgen/core/DistanceMapping.h>
#include <msdfgen/core/ShapeDistanceFinder.h>
#include <msdfgen/core/edge-selectors.h>

#include "Duo2D/arith/rect.hpp"


namespace d2d::decoder {
	result<llfio::mapped_file_handle> open_file(llfio::path_view path) noexcept {
        auto mf = llfio::mapped_file({}, path);
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());

		return sl::move(mh);
	}
}

namespace d2d::decoder { 
	result<texture>
	decode_texture(llfio::mapped_file_handle const& handle, texture_usage usage) noexcept {
		using texture_unique_ptr = sl::unique_ptr<ktxTexture2, sl::functor::generic_stateless<ktxTexture2_Destroy>>;
		texture_unique_ptr ktx_ptr = sl::make_default<texture_unique_ptr>(sl::in_place_tag);
        __D2D_KTX_VERIFY(ktxTexture2_CreateFromMemory(reinterpret_cast<ktx_uint8_t const*>(handle.address()), handle.maximum_extent().assume_value(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx_ptr.get()));


        //TODO: compressed format transcoding
        const bool transcoded = ktxTexture2_NeedsTranscoding(ktx_ptr.get());
        if (transcoded) __D2D_KTX_VERIFY(ktxTexture2_TranscodeBasis(ktx_ptr.get(), KTX_TTF_RGBA32, 0));

		std::vector<sl::byte> bytes(ktx_ptr->dataSize);
		std::memcpy(bytes.data(), ktx_ptr->pData, ktx_ptr->dataSize);

		sl::array<max_mip_levels, sl::uoffset_t> mip_offsets;
		for(sl::index_t i = 0; i < std::min(static_cast<ktx_uint32_t>(max_mip_levels), ktx_ptr->numLevels); ++i)
			__D2D_KTX_VERIFY(ktxTexture2_GetImageOffset(ktx_ptr.get(), i, 0, 0, &mip_offsets[i]));


		return texture{
			texture_info{
				.dimensions = ktx_ptr->numDimensions,
				.format_id = ktxTexture2_GetVkFormat(ktx_ptr.get()),
				.extent = {ktx_ptr->baseWidth, ktx_ptr->baseHeight, ktx_ptr->baseDepth},
				.mip_level_count = ktx_ptr->numLevels,
				.layer_count = ktx_ptr->numLayers,
				.sample_count = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = usage,
				.mip_offsets = mip_offsets
			},
			sl::move(bytes)
		};
	}

	result<std::vector<std::array<std::byte, font_texture::size_bytes>>>
	decode_font(llfio::mapped_file_handle const& handle) noexcept {
        std::span<const std::byte> font_file_bytes(reinterpret_cast<std::byte const*>(handle.address()), handle.maximum_extent().assume_value());
        sl::unique_ptr<hb_blob_t, sl::functor::generic_stateless<hb_blob_destroy>> blob_ptr(hb_blob_create(reinterpret_cast<char const*>(font_file_bytes.data()), font_file_bytes.size(), HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr));
        sl::unique_ptr<hb_face_t, sl::functor::generic_stateless<hb_face_destroy>> face_ptr(hb_face_create(blob_ptr.get(), 0));
        sl::unique_ptr<hb_font_t, sl::functor::generic_stateless<hb_font_destroy>> font_ptr(hb_font_create(face_ptr.get()));

        const unsigned int units_per_em = hb_face_get_upem(face_ptr.get());
        const double scale = 1./units_per_em;
        impl::glyph_context glyph_ctx{
            .pos = {},
            .scale = scale,
        };

        hb_draw_funcs_t* draw_funcs = hb_draw_funcs_create();
        hb_draw_funcs_set_move_to_func     (draw_funcs, ::d2d::impl::move_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_line_to_func     (draw_funcs, ::d2d::impl::line_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_quadratic_to_func(draw_funcs, ::d2d::impl::quad_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_cubic_to_func    (draw_funcs, ::d2d::impl::cubic_to, &glyph_ctx, nullptr);

        const unsigned int glyph_count = hb_face_get_glyph_count(face_ptr.get());
        std::vector<std::array<std::byte, font_texture::size_bytes>> glyphs(glyph_count);
        std::vector<rect<float>> glyph_bounds(glyph_count);
        std::vector<pt2f> glyph_padding(glyph_count);
        errc ret = errc::unknown;

        //#pragma omp taskloop 
        for(unsigned int glyph_id = 0; glyph_id < glyph_count; ++glyph_id){
            msdfgen::Shape shape;
            if(!hb_font_draw_glyph_or_fail(font_ptr.get(), glyph_id, draw_funcs, &shape)) [[unlikely]] {
                //#pragma omp atomic write
                ret = errc::invalid_font_file_format;
                break;
                //#pragma omp cancel taskgroup
            }
            //#pragma omp cancellation point taskgroup

            if (!shape.contours.empty() && shape.contours.back().edges.empty())
                shape.contours.pop_back();
            shape.inverseYAxis = true;
            shape.normalize();
            msdfgen::edgeColoringSimple(shape, 3.0);
            msdfgen::Shape::Bounds b = shape.getBounds();
            constexpr static double font_texture_length_em = font_texture::length_pixels / 16.0;
            pt2f top_left{
                static_cast<float>(std::clamp(b.l, -font_texture_length_em, font_texture_length_em)), 
                static_cast<float>(std::clamp(b.t, -font_texture_length_em, font_texture_length_em))
            };
            pt2f bottom_right{
                static_cast<float>(std::clamp(b.r, -font_texture_length_em, font_texture_length_em)),
                static_cast<float>(std::clamp(b.b, -font_texture_length_em, font_texture_length_em)),
            };
            glyph_bounds[glyph_id] = {
                top_left.x(), top_left.y(), 
                bottom_right.x() - top_left.x(), bottom_right.y() - top_left.y()
            };

            std::array<float, font_texture::size_bytes> bitmap;

            //msdfgen::DistanceMapping mapping((msdfgen::Range(font_texture_distance_range)));
            //TODO: true SIMD
            glyph_padding[glyph_id] = static_cast<float>(font_texture::padding_em) - pt2f{top_left.x(), bottom_right.y()};
            pt2f msdf_padding = glyph_padding[glyph_id];//font_texture_padding_em - pt2d{top_left.x(), bottom_right.y()};
            #pragma omp parallel for collapse(2)
            for (std::size_t y = 0; y < font_texture::length_pixels; ++y) {
                for (std::size_t col = 0; col < font_texture::length_pixels; ++col) {
                    std::size_t x = (y % 2) ? font_texture::length_pixels - col - 1 : col;
                    pt2d p = pt2d{x + .5, y + .5} / font_texture::glyph_scale - msdf_padding;//pt2d{bitmap_padding_em, ((bitmap_pixel_length)/ static_cast<double>(glyph_scale)) - (b.t - b.b) - bitmap_padding_em};
                    msdfgen::ShapeDistanceFinder<msdfgen::OverlappingContourCombiner<msdfgen::MultiAndTrueDistanceSelector>> distanceFinder(shape);
                    std::array<double, font_texture::channels> distance = std::bit_cast<std::array<double, font_texture::channels>>(distanceFinder.distance(std::bit_cast<msdfgen::Point2>(p)));
                    float* const bitmap_begin = &bitmap[font_texture::channels * (font_texture::length_pixels * (font_texture::length_pixels - y - 1) + x)];
                    for(std::size_t channel = 0; channel < font_texture::channels; ++channel)
                        bitmap_begin[channel] = static_cast<float>(distance[channel] / font_texture::distance_range + .5);
                }
            }

            msdfErrorCorrection(msdfgen::BitmapRef<float, 4>{bitmap.data(), font_texture::length_pixels, font_texture::length_pixels}, shape, msdfgen::Projection(font_texture::glyph_scale, msdfgen::Vector2(msdf_padding.x(), msdf_padding.y())), msdfgen::Range(font_texture::distance_range));

            for(std::size_t i = 0; i < font_texture::size_bytes; ++i)
                glyphs[glyph_id][i] = static_cast<std::byte>(msdfgen::pixelFloatToByte(bitmap[i]));
        }
        if(ret != errc::unknown) return ret;

		return sl::move(glyphs);
	}
}



namespace d2d::impl {
    constexpr void move_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept {
        //TODO: use hb_draw_state instead of glyph_context->pos
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        if(shape_ptr->contours.empty() || !shape_ptr->contours.back().edges.empty())
            shape_ptr->contours.emplace_back();
        glyph_data_ptr->pos = pt2f{target_x, target_y} * glyph_data_ptr->scale;
    };
    
    constexpr void line_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        if (old_val != glyph_data_ptr->pos)
            shape_ptr->contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos));
    };
    
    constexpr void quad_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float control_x, float control_y, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        if (old_val != glyph_data_ptr->pos) {
            const pt2d control_val = pt2f{control_x, control_y} * glyph_data_ptr->scale;
            shape_ptr->contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(control_val), std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos));
        }
    };
    
    constexpr void cubic_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float first_control_x, float first_control_y, float second_control_x, float second_control_y, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        const pt2d first_control_val = pt2f{first_control_x, first_control_y} * glyph_data_ptr->scale;
        const pt2d second_control_val = pt2f{second_control_x, second_control_y} * glyph_data_ptr->scale;
        if (old_val != glyph_data_ptr->pos || d2d::cross(first_control_val, second_control_val) != 0.) {
            shape_ptr->contours.back().edges.emplace_back(
                std::bit_cast<msdfgen::Point2>(old_val), 
                std::bit_cast<msdfgen::Point2>(first_control_val), 
                std::bit_cast<msdfgen::Point2>(second_control_val), 
                std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos)
            );
        }
    }; 
}