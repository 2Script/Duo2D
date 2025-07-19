#include "Duo2D/vulkan/memory/texture_map.hpp"

#include <bit>
#include <iterator>
#include <memory>
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <msdfgen/core/DistanceMapping.h>
#include <msdfgen/core/ShapeDistanceFinder.h>
#include <msdfgen/core/edge-selectors.h>
#include <numeric>
#include <cstdint>
#include <result/verify.h>
#include <span>
#include <string_view>
#include <tuple>
#include <llfio.hpp>
#include <harfbuzz/hb.h>
#include <DuoDecode/decoder.hpp>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/display/pixel_format_mapping.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/core/make.hpp"

namespace d2d::vk {
    result<texture_idx_t> texture_map::load(std::string_view path, std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map>, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
        std::pair<iterator, bool> emplace_result = emplace(std::piecewise_construct, std::forward_as_tuple(path), std::forward_as_tuple());
        texture& tex = emplace_result.first->second;
        if(!emplace_result.second) return tex.index();
        

        namespace llfio = LLFIO_V2_NAMESPACE;
        auto mf = llfio::mapped_file({}, llfio::path_view(path));
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());
        std::size_t length = extent.assume_value();

        dd::decoder image_decoder(mh.address(), length);
        RESULT_TRY_MOVE_UNSCOPED_CAST_ERR(dd::decoded_video decoded_image, image_decoder.decode_video_only(AV_HWDEVICE_TYPE_VULKAN), di, d2d::errc);
        std::array<std::span<const std::byte>, 1> bytes_arr{std::span{decoded_image.bytes}};


        return create_texture(
            emplace_result.first, std::span{bytes_arr}, {decoded_image.width, decoded_image.height}, pixel_format_mapping[decoded_image.format],
            logi_device, phys_device, copy_cmd_pool, texture_mem
        );
    }


    result<texture_idx_t> texture_map::load(const font& f, std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_map, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
        std::pair<iterator, bool> emplace_result = emplace(std::piecewise_construct, std::forward_as_tuple(f.key()), std::forward_as_tuple());
        texture& tex = emplace_result.first->second;
        if(!emplace_result.second) return tex.index();
        

        //TODO (HIGH PRIO): custom font creation (msdfgen has no customization whatsoever and a terrible interface)
        namespace llfio = LLFIO_V2_NAMESPACE;
        auto mf = llfio::mapped_file({}, llfio::path_view(f.path()));
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());
        std::size_t length = extent.assume_value();
        const std::span<const std::byte> font_bytes{reinterpret_cast<std::byte const*>(mh.address()), length};


        auto font_data_iter = font_map->try_emplace(emplace_result.first->first, font_bytes);
        ::d2d::impl::font_data& font_data = font_data_iter.first->second;

        if(auto close = mh.close(); close.has_error())
            return static_cast<errc>(close.assume_error().value());


        const unsigned int units_per_em = hb_face_get_upem(font_data.face_ptr.get());
        const double scale = 1./units_per_em;
        impl::glyph_context glyph_ctx{
            .pos = {},
            .scale = scale,
        };

        hb_draw_funcs_t* draw_funcs = hb_draw_funcs_create();
        hb_draw_funcs_set_move_to_func     (draw_funcs, impl::draw_op::move_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_line_to_func     (draw_funcs, impl::draw_op::line_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_quadratic_to_func(draw_funcs, impl::draw_op::quad_to,  &glyph_ctx, nullptr);
        hb_draw_funcs_set_cubic_to_func    (draw_funcs, impl::draw_op::cubic_to, &glyph_ctx, nullptr);


        const unsigned int glyph_count = hb_face_get_glyph_count(font_data.face_ptr.get());
        hb_font_t* font_ptr = font_data.font_ptr.get();
        std::vector<std::array<std::byte, font_texture_size_bytes>> glyphs(glyph_count);
        ::d2d::impl::glyph_bounds_vector glyph_bounds(glyph_count);
        std::vector<pt2f> glyph_padding(glyph_count);
        errc ret = errc::unknown;

        //#pragma omp taskloop 
        for(unsigned int glyph_id = 0; glyph_id < glyph_count; ++glyph_id){
            msdfgen::Shape shape;
            if(!hb_font_draw_glyph_or_fail(font_ptr, glyph_id, draw_funcs, &shape)) [[unlikely]] {
                //#pragma omp atomic write
                ret = errc::bad_font_file_format;
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
            constexpr static double font_texture_length_em = font_texture_length_pixels / 16.0;
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

            std::array<float, font_texture_size_bytes> bitmap;

            //msdfgen::DistanceMapping mapping((msdfgen::Range(font_texture_distance_range)));
            //TODO: true SIMD
            glyph_padding[glyph_id] = static_cast<float>(font_texture_padding_em) - pt2f{top_left.x(), bottom_right.y()};
            pt2f msdf_padding = glyph_padding[glyph_id];//font_texture_padding_em - pt2d{top_left.x(), bottom_right.y()};
            #pragma omp parallel for collapse(2)
            for (std::size_t y = 0; y < font_texture_length_pixels; ++y) {
                for (std::size_t col = 0; col < font_texture_length_pixels; ++col) {
                    std::size_t x = (y % 2) ? font_texture_length_pixels - col - 1 : col;
                    pt2d p = pt2d{x + .5, y + .5} / font_texture_glyph_scale - msdf_padding;//pt2d{bitmap_padding_em, ((bitmap_pixel_length)/ static_cast<double>(glyph_scale)) - (b.t - b.b) - bitmap_padding_em};
                    msdfgen::ShapeDistanceFinder<msdfgen::OverlappingContourCombiner<msdfgen::MultiAndTrueDistanceSelector>> distanceFinder(shape);
                    std::array<double, font_texture_channels> distance = std::bit_cast<std::array<double, font_texture_channels>>(distanceFinder.distance(std::bit_cast<msdfgen::Point2>(p)));
                    float* const bitmap_begin = &bitmap[font_texture_channels * (font_texture_length_pixels * (font_texture_length_pixels - y - 1) + x)];
                    for(std::size_t channel = 0; channel < font_texture_channels; ++channel)
                        bitmap_begin[channel] = static_cast<float>(distance[channel] / font_texture_distance_range + .5);
                }
            }

            msdfErrorCorrection(msdfgen::BitmapRef<float, 4>{bitmap.data(), font_texture_length_pixels, font_texture_length_pixels}, shape, msdfgen::Projection(font_texture_glyph_scale, msdfgen::Vector2(msdf_padding.x(), msdf_padding.y())), msdfgen::Range(font_texture_distance_range));

            for(std::size_t i = 0; i < font_texture_size_bytes; ++i)
                glyphs[glyph_id][i] = static_cast<std::byte>(msdfgen::pixelFloatToByte(bitmap[i]));
        }
        if(ret != errc::unknown) return ret;


        font_data.glyph_bounds = std::move(glyph_bounds);
        font_data.glyph_padding = std::move(glyph_padding);
        std::vector<std::span<const std::byte>> glyph_spans(glyphs.size());
        for(std::size_t i = 0; i < glyphs.size(); ++i)
            glyph_spans[i] = std::span{reinterpret_cast<std::byte const*>(glyphs[i].data()), glyphs[i].size()};


        return create_texture(
            emplace_result.first, std::span{glyph_spans}, {font_texture_length_pixels, font_texture_length_pixels}, VK_FORMAT_R8G8B8A8_UNORM,
            logi_device, phys_device, copy_cmd_pool, texture_mem
        );
    }
}

namespace d2d::vk {    
    result<texture_idx_t> texture_map::create_texture(iterator& tex_iter, std::span<std::span<const std::byte>> textures_as_bytes, extent2 texture_size, VkFormat format, std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
        texture& tex = tex_iter->second;
        RESULT_TRY_MOVE(tex, make<texture>(
            logi_device, 
            texture_size.width(), texture_size.height(), format, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            textures_as_bytes.size()
        ));

        renderable_allocator allocator(logi_device, phys_device, copy_cmd_pool);

        const std::size_t total_buffer_size = std::accumulate(textures_as_bytes.begin(), textures_as_bytes.end(), 0, [](std::size_t sum, std::span<const std::byte> bytes){ return sum + bytes.size_bytes(); });
        RESULT_VERIFY_UNSCOPED(allocator.stage(total_buffer_size, textures_as_bytes), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        
        //Re-allocate the memory
        device_memory<std::dynamic_extent> old_mem = std::move(texture_mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
        RESULT_TRY_MOVE(texture_mem, make<device_memory<std::dynamic_extent>>(logi_device, phys_device, *this, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
         
        //Re-create all other textures
        RESULT_VERIFY(allocator.gpu_alloc_begin());
        std::vector<texture> new_texs;
        new_texs.resize(size());
        RESULT_VERIFY(allocator.realloc(new_texs, *this, texture_mem, std::distance(begin(), tex_iter)));
        RESULT_VERIFY(allocator.gpu_alloc_end());

        //Replace the old textures with the new ones
        allocator.move(*this, new_texs);
        
        //Create copy regions from staging buffer to device local buffer
        std::vector<VkBufferImageCopy> copy_regions;
        copy_regions.reserve(textures_as_bytes.size());
        const std::size_t texture_size_bytes = tex.size_bytes();
        for(std::size_t i = 0; i < textures_as_bytes.size(); ++i) {
            if(textures_as_bytes[i].size_bytes() != texture_size_bytes)
                return errc::invalid_image_initialization;

            copy_regions.emplace_back(
                static_cast<std::uint32_t>(texture_size_bytes * i), //bufferOffset
                static_cast<std::uint32_t>(0), //bufferRowLength
                static_cast<std::uint32_t>(0), //bufferImageHeight

                VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, static_cast<std::uint32_t>(i), 1}, //imageSubresource
                VkOffset3D{0, 0, 0}, //imageOffset
                VkExtent3D{texture_size.width(), texture_size.height(), 1} //imageExtent
            );
        }

        //Create copy command buffer
        RESULT_VERIFY(allocator.gpu_alloc_begin());
        allocator.copy_command_buffer().transition_image(tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, textures_as_bytes.size());
        allocator.copy_command_buffer().copy_buffer_to_image(tex, staging_buffer, std::span{copy_regions});
        allocator.copy_command_buffer().transition_image(tex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textures_as_bytes.size());
        //Clear copy command buffer
        RESULT_VERIFY(allocator.gpu_alloc_end());

        RESULT_VERIFY(tex.initialize(logi_device, phys_device, format));
        return tex.index();
    }
}