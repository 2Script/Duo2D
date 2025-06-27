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
#include <DuoDecode/decoder.hpp>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/display/pixel_format_mapping.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/core/make.hpp"
#include FT_OUTLINE_H

namespace d2d::vk {
    result<texture_idx_t> texture_map::load(std::string_view path, logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
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


    result<texture_idx_t> texture_map::load(const font& f, logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
        std::string key("font::");
        key.append(f.name());
        std::pair<iterator, bool> emplace_result = emplace(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple());
        texture& tex = emplace_result.first->second;
        if(!emplace_result.second) return tex.index();


        constexpr static std::size_t printable_ascii_count = 0x7F - 0x20;
        constexpr static std::size_t bitmap_pixel_length = 32;
        constexpr static double bitmap_em_scale = 0.125;
        constexpr static std::size_t bitmap_channels = 4;
        constexpr static std::size_t bitmap_size = bitmap_pixel_length * bitmap_pixel_length * bitmap_channels;
        

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

        font ret{};
        RESULT_VERIFY(freetype_init.initialize(impl::texture_map_count(), []() noexcept -> result<FT_Library> {
            FT_Library library;
            __D2D_FT_VERIFY(FT_Init_FreeType(&library));
            return library;
        }));

        FT_Open_Args font_open_args {
            .flags = FT_OPEN_MEMORY,
            .memory_base = reinterpret_cast<FT_Byte*>(mh.address()),
            .memory_size = static_cast<FT_Long>(length)
        };

        FT_Face face_ptr;
        __D2D_FT_VERIFY(FT_Open_Face(decltype(freetype_init)::data, &font_open_args, 0, &face_ptr));
        std::unique_ptr<std::remove_pointer_t<FT_Face>, generic_functor<FT_Done_Face>> face(face_ptr, {});


        constexpr auto move_to_fn = [](FT_Vector const* target, void* ctx) noexcept -> FT_Error {
            freetype_context* ft_ctx = static_cast<freetype_context*>(ctx);
            if(ft_ctx->shape.contours.empty() || !ft_ctx->shape.contours.back().edges.empty())
                ft_ctx->shape.contours.emplace_back();
            ft_ctx->pos = pt2<FT_Pos>{target->x, target->y} * ft_ctx->scale;
            return FT_Err_Ok;
        };
        
        constexpr auto line_to_fn = [](FT_Vector const* target, void* ctx) noexcept -> FT_Error {
            freetype_context* ft_ctx = static_cast<freetype_context*>(ctx);
            const pt2d old_val = std::exchange(ft_ctx->pos, pt2<FT_Pos>{target->x, target->y} * ft_ctx->scale);
            if (old_val != ft_ctx->pos)
                ft_ctx->shape.contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(ft_ctx->pos));
            return FT_Err_Ok;
        };
        
        constexpr auto conic_to_fn = [](FT_Vector const* control, FT_Vector const* target, void* ctx) noexcept -> FT_Error {
            freetype_context* ft_ctx = static_cast<freetype_context*>(ctx);
            const pt2d old_val = std::exchange(ft_ctx->pos, pt2<FT_Pos>{target->x, target->y} * ft_ctx->scale);
            if (old_val != ft_ctx->pos) {
                const pt2d control_val = pt2<FT_Pos>{control->x, control->y} * ft_ctx->scale;
                ft_ctx->shape.contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(control_val), std::bit_cast<msdfgen::Point2>(ft_ctx->pos));
            }
            return FT_Err_Ok;
        };
        
        constexpr auto cubic_to_fn = [](FT_Vector const* first_control, FT_Vector const* second_control, FT_Vector const* target, void* ctx) noexcept -> FT_Error {
            freetype_context* ft_ctx = static_cast<freetype_context*>(ctx);
            const pt2d old_val = std::exchange(ft_ctx->pos, pt2<FT_Pos>{target->x, target->y} * ft_ctx->scale);
            const pt2d first_control_val = pt2<FT_Pos>{first_control->x, first_control->y} * ft_ctx->scale;
            const pt2d second_control_val = pt2<FT_Pos>{second_control->x, second_control->y} * ft_ctx->scale;
            if (old_val != ft_ctx->pos || d2d::cross(first_control_val, second_control_val) != 0.) {
                ft_ctx->shape.contours.back().edges.emplace_back(
                    std::bit_cast<msdfgen::Point2>(old_val), 
                    std::bit_cast<msdfgen::Point2>(first_control_val), 
                    std::bit_cast<msdfgen::Point2>(second_control_val), 
                    std::bit_cast<msdfgen::Point2>(ft_ctx->pos)
                );
            }
            return FT_Err_Ok;
        };

        FT_Outline_Funcs outline_funcs {
            .move_to = move_to_fn,
            .line_to = line_to_fn,
            .conic_to = conic_to_fn,
            .cubic_to = cubic_to_fn,
            .shift = 0,
            .delta = 0
        };

        std::array<std::array<std::byte, bitmap_size>, printable_ascii_count> glyphs{};
        for(char c = ' '; c < '\x7f'; ++c){
            msdfgen::Shape shape;
            
            __D2D_FT_VERIFY(FT_Load_Glyph(face.get(), FT_Get_Char_Index(face.get(), c), FT_LOAD_NO_SCALE));
            freetype_context ft_ctx {
                .pos = {},
                .scale = 1./(face->units_per_EM ? face->units_per_EM : 1),
                .shape = shape
            };
            
            __D2D_FT_VERIFY(FT_Outline_Decompose(&(face->glyph->outline), &outline_funcs, &ft_ctx));
            if (!shape.contours.empty() && shape.contours.back().edges.empty())
                shape.contours.pop_back();
            shape.inverseYAxis = true;
            shape.normalize();
            msdfgen::edgeColoringSimple(shape, 3.0);

            msdfgen::ShapeDistanceFinder<msdfgen::OverlappingContourCombiner<msdfgen::MultiAndTrueDistanceSelector>> distanceFinder(shape);
            bool rtl = false;
            std::array<std::byte, bitmap_size>& bytes = glyphs[c - ' '];
            std::array<float, bitmap_size> bitmap;

            msdfgen::DistanceMapping mapping((msdfgen::Range(bitmap_em_scale)));
            //TODO simd and parallelism
            for (std::size_t y = 0; y < bitmap_pixel_length; ++y) {
                std::size_t row = bitmap_pixel_length - y - 1;
                for (std::size_t col = 0; col < bitmap_pixel_length; ++col) {
                    std::size_t x = rtl ? bitmap_pixel_length - col - 1 : col;
                    pt2d p = pt2d{x + .5, y + .5} / bitmap_pixel_length - bitmap_em_scale;
                    std::array<double, bitmap_channels> distance = std::bit_cast<std::array<double, bitmap_channels>>(distanceFinder.distance(std::bit_cast<msdfgen::Point2>(p)));
                    float* const bitmap_begin = &bitmap[bitmap_channels * (bitmap_pixel_length * row + x)];
                    for(std::size_t channel = 0; channel < bitmap_channels; ++channel)
                        bitmap_begin[channel] = static_cast<float>(distance[channel] / bitmap_em_scale + .5);
                }
                rtl = !rtl;
            }

            msdfErrorCorrection(msdfgen::BitmapRef<float, 4>{bitmap.data(), bitmap_pixel_length, bitmap_pixel_length}, shape, msdfgen::Projection(bitmap_pixel_length, msdfgen::Vector2(bitmap_em_scale, bitmap_em_scale)), msdfgen::Range(bitmap_em_scale));
            for(std::size_t i = 0; i < bitmap_size; ++i)
                bytes[i] = static_cast<std::byte>(msdfgen::pixelFloatToByte(bitmap[i])); //TODO Why not just multiply by 255?

        }

        faces.emplace(emplace_result.first->first, std::move(face));

        std::array<std::span<const std::byte>, printable_ascii_count> glyph_spans{};
        for(std::size_t i = 0; i < printable_ascii_count; ++i)
            glyph_spans[i] = std::span{reinterpret_cast<std::byte const*>(glyphs[i].data()), glyphs[i].size()};

        return create_texture(
            emplace_result.first, std::span{glyph_spans}, {bitmap_pixel_length, bitmap_pixel_length}, VK_FORMAT_R8G8B8A8_UNORM,
            logi_device, phys_device, copy_cmd_pool, texture_mem
        );
    }
}

namespace d2d::vk {    
    result<texture_idx_t> texture_map::create_texture(iterator& tex_iter, std::span<std::span<const std::byte>> textures_as_bytes, extent2 texture_size, VkFormat format, logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept {
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


// namespace d2d::vk {
//     impl::instance_tracker<msdfgen::FreetypeHandle*, msdfgen::deinitializeFreetype> texture_map::freetype_init{};
// }