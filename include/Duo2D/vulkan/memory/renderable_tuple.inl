#pragma once
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"

#include <memory>
#include <numeric>
#include <string_view>

#include <result.hpp>
#include <vulkan/vulkan.h>
#include <llfio.hpp>
#include <DuoDecode/decoder.hpp>
#include <msdfgen.h>
#include <msdfgen/core/DistanceMapping.h>
#include <msdfgen/core/ShapeDistanceFinder.h>
#include <msdfgen/core/edge-selectors.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/traits/buffer_traits.hpp"
#include "Duo2D/traits/renderable_properties.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/display/pixel_format_mapping.hpp"


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    result<renderable_tuple<FiF, std::tuple<Ts...>>> renderable_tuple<FiF, std::tuple<Ts...>>::create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_data_map, render_pass& window_render_pass) noexcept {
        renderable_tuple ret{};
        ret.logi_device_ptr = logi_device;
        ret.phys_device_ptr = phys_device;
        ret.font_data_map_ptr = font_data_map;
        
        //Create command pool for copy commands
        vk::command_pool c;
        RESULT_TRY_MOVE(c, make<command_pool>(logi_device, phys_device));
        ret.copy_cmd_pool_ptr = std::make_shared<command_pool>(std::move(c));

        //Create allocator
        renderable_allocator allocator(logi_device, phys_device, ret.copy_cmd_pool_ptr);

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = ((ret.template renderable_data_of<Ts>().static_index_data_bytes.size()) + ...) + ((ret.template renderable_data_of<Ts>().static_vertex_data_bytes.size()) + ...);
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        //Create the compile-time static input data for instanced types
        constexpr static std::size_t static_data_count = ((renderable_constraints<Ts>::instanced ? renderable_constraints<Ts>::has_fixed_indices + renderable_constraints<Ts>::has_fixed_vertices : 0) + ...);
        constexpr static std::array<std::span<const std::byte>, static_data_count> inputs = [](){
            std::array<std::span<const std::byte>, static_data_count> bytes;
            std::size_t idx = 0;
            constexpr auto emplace_instanced_input = []<typename T>(std::array<std::span<const std::byte>, static_data_count>& arr, std::size_t& i){
                if constexpr (renderable_constraints<T>::instanced && renderable_constraints<T>::has_fixed_indices)  arr[i++] = std::span<const std::byte>(renderable_data<T, FiF>::static_index_data_bytes);
                if constexpr (renderable_constraints<T>::instanced && renderable_constraints<T>::has_fixed_vertices) arr[i++] = std::span<const std::byte>(renderable_data<T, FiF>::static_vertex_data_bytes);
            };
            (emplace_instanced_input.template operator()<Ts>(bytes, idx), ...);
            return bytes;
        }();

        //Stage the static input data and allocate it to device-local memory
        RESULT_VERIFY_UNSCOPED(allocator.stage(static_buffer_size, inputs), s);
        auto [staging_buffer, staging_mem] = *std::move(s);

        RESULT_VERIFY((allocator.template alloc_buffer<0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.static_data_buff), 1}, static_buffer_size, ret.static_device_local_mem
        )));
        RESULT_VERIFY(allocator.staging_to_device_local(ret.static_data_buff, staging_buffer));
        goto create_uniform;
        }

    create_uniform:
        //If theres no uniform data, skip creating its buffer
        constexpr static std::size_t uniform_buffer_size = (renderable_data<Ts, FiF>::uniform_data_size + ...);
        if constexpr (uniform_buffer_size == 0) goto create_descriptors;

        //Create uniform buffer
        RESULT_VERIFY((allocator.template alloc_buffer<0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.uniform_buff), 1}, uniform_buffer_size, ret.host_mem
        )));

        RESULT_TRY_COPY(ret.uniform_buffer_map, ret.host_mem.map(logi_device, uniform_buffer_size));
        goto create_descriptors;

    create_descriptors:
        //Create pipelines and descriptors
        //TODO: Simplify this
        errc error_code = error::unknown;
        auto create_descriptors = [&]<typename T>(errc& current_error_code) noexcept -> errc {
            if(current_error_code != error::unknown) return current_error_code;
            RESULT_VERIFY(ret.template renderable_data_of<T>().create_uniform_descriptors(ret.uniform_buff, static_offsets<T>()[buffer_data_type::uniform]));
            RESULT_VERIFY(ret.template renderable_data_of<T>().create_texture_descriptors(ret.textures));
            RESULT_VERIFY(ret.template renderable_data_of<T>().create_pipeline_layout(logi_device));
            RESULT_VERIFY(ret.template renderable_data_of<T>().create_pipeline(logi_device, window_render_pass));
            return error::unknown;
        };
        ((error_code = create_descriptors.template operator()<Ts>(error_code)), ...);
        if(error_code != error::unknown) return error_code;

        return ret;
    }
}

namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_tuple<FiF, std::tuple<Ts...>>::apply_changes(render_pass& window_render_pass) noexcept {
        constexpr static std::size_t I = renderable_index<T>;
        if(this->template renderable_data_of<T>().size() == 0) {
            data_buffs[I] = buffer{};
            return {};
        }
        
        renderable_allocator allocator(logi_device_ptr, phys_device_ptr, copy_cmd_pool_ptr);
        auto load_texture = [this]<typename K>(K&& key) noexcept -> auto {
            return this->load(std::forward<K>(key));
        };
        
        //Get the input data bytes, stage them, then move them to device local memory
        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> input_bytes, this->template renderable_data_of<T>().make_inputs(load_texture), ib);
        RESULT_VERIFY_UNSCOPED(allocator.stage(this->template renderable_data_of<T>().input_size(), input_bytes), s);
        this->template renderable_data_of<T>().clear_inputs();
        auto [staging_buffer, staging_mem] = *std::move(s);

        if(this->template renderable_data_of<T>().input_size() > data_buffs[I].size()) 
            RESULT_VERIFY((allocator.template alloc_buffer<I, renderable_data<T, FiF>::input_usage_flags(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(std::span{data_buffs}, this->template renderable_data_of<T>().input_size(), device_local_mem)));
        RESULT_VERIFY((allocator.staging_to_device_local(data_buffs[I], staging_buffer)));

        if constexpr (renderable_constraints<T>::has_textures) {
            //TODO: Simplify this
            errc error_code = error::unknown;
            auto update_indicies = [&]<typename U>(errc& current_error_code) noexcept -> errc {
                if constexpr(!renderable_constraints<U>::has_textures) return current_error_code;
                else {
                    if(current_error_code != error::unknown) return current_error_code;
                    RESULT_VERIFY(this->template renderable_data_of<U>().template update_texture_indices<T>(load_texture, allocator, data_buffs[renderable_index<U>]));
                    RESULT_VERIFY(this->template renderable_data_of<U>().create_texture_descriptors(textures));
                    RESULT_VERIFY(this->template renderable_data_of<U>().create_pipeline_layout(logi_device_ptr));
                    RESULT_VERIFY(this->template renderable_data_of<U>().create_pipeline(logi_device_ptr, window_render_pass));
                    return error::unknown;
                }
            };
            ((error_code = update_indicies.template operator()<Ts>(error_code)), ...);
            (this->template renderable_data_of<Ts>().clear_inputs(), ...);
            if(error_code != error::unknown) return error_code;
        }
        
        if constexpr (!renderable_constraints<T>::has_attributes) {
            this->template renderable_data_of<T>().has_changes() = false;
            return {};
        }

        constexpr static std::size_t I_a = renderable_index_with_attrib<T>;
        (this->template renderable_data_of<Ts>().unbind_attributes(), ...);

        if(this->template renderable_data_of<T>().attribute_buffer_size() == 0) {
            attribute_buffs[I_a] = buffer{};
            return {};
        }
        
        if(this->template renderable_data_of<T>().attribute_buffer_size() > attribute_buffs[I_a].size()) {
            RESULT_VERIFY((allocator.template alloc_buffer<I_a, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT>(
                std::span{attribute_buffs}, this->template renderable_data_of<T>().attribute_buffer_size(), shared_mem
            )));
        }

        //Must recreate ALL attribute span maps
        RESULT_TRY_COPY_UNSCOPED(void* shared_mem_map, shared_mem.map(logi_device_ptr, VK_WHOLE_SIZE), smm);
        std::size_t buffer_offset = 0;

        (this->template renderable_data_of<Ts>().emplace_attributes(buffer_offset, shared_mem_map, shared_mem.requirements()[renderable_index_with_attrib<Ts>].size), ...);

        this->template renderable_data_of<T>().has_changes() = false;
        return {};
    }
}



namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    result<texture_idx_t> renderable_tuple<FiF, std::tuple<Ts...>>::load(std::string_view path) noexcept {
        std::pair<typename texture_map::iterator, bool> emplace_result = textures.emplace(std::piecewise_construct, std::forward_as_tuple(path), std::forward_as_tuple());
        if(!emplace_result.second) return emplace_result.first->second.index();
        

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


        return create_texture(emplace_result.first, std::span{bytes_arr}, {decoded_image.width, decoded_image.height}, pixel_format_mapping[decoded_image.format]);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    result<texture_idx_t> renderable_tuple<FiF, std::tuple<Ts...>>::load(font_view f, std::string_view path) noexcept {
        std::pair<typename texture_map::iterator, bool> emplace_result = textures.emplace(std::piecewise_construct, std::forward_as_tuple(f), std::forward_as_tuple());
        if(!emplace_result.second) return emplace_result.first->second.index();
        if(path.empty()) [[unlikely]] {
            textures.erase(emplace_result.first);
            return errc::font_not_found;
        }
        

        //TODO (HIGH PRIO): custom font creation (msdfgen has no customization whatsoever and a terrible interface)
        namespace llfio = LLFIO_V2_NAMESPACE;
        auto mf = llfio::mapped_file({}, llfio::path_view(path));
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());
        std::size_t length = extent.assume_value();
        const std::span<const std::byte> font_bytes{reinterpret_cast<std::byte const*>(mh.address()), length};


        auto font_data_iter = font_data_map_ptr->try_emplace(emplace_result.first->first, font_bytes);
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
        std::vector<std::array<std::byte, texture_map::font_texture_size_bytes>> glyphs(glyph_count);
        std::vector<rect<float>> glyph_bounds(glyph_count);
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
            constexpr static double font_texture_length_em = texture_map::font_texture_length_pixels / 16.0;
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

            std::array<float, texture_map::font_texture_size_bytes> bitmap;

            //msdfgen::DistanceMapping mapping((msdfgen::Range(font_texture_distance_range)));
            //TODO: true SIMD
            glyph_padding[glyph_id] = static_cast<float>(texture_map::font_texture_padding_em) - pt2f{top_left.x(), bottom_right.y()};
            pt2f msdf_padding = glyph_padding[glyph_id];//font_texture_padding_em - pt2d{top_left.x(), bottom_right.y()};
            #pragma omp parallel for collapse(2)
            for (std::size_t y = 0; y < texture_map::font_texture_length_pixels; ++y) {
                for (std::size_t col = 0; col < texture_map::font_texture_length_pixels; ++col) {
                    std::size_t x = (y % 2) ? texture_map::font_texture_length_pixels - col - 1 : col;
                    pt2d p = pt2d{x + .5, y + .5} / texture_map::font_texture_glyph_scale - msdf_padding;//pt2d{bitmap_padding_em, ((bitmap_pixel_length)/ static_cast<double>(glyph_scale)) - (b.t - b.b) - bitmap_padding_em};
                    msdfgen::ShapeDistanceFinder<msdfgen::OverlappingContourCombiner<msdfgen::MultiAndTrueDistanceSelector>> distanceFinder(shape);
                    std::array<double, texture_map::font_texture_channels> distance = std::bit_cast<std::array<double, texture_map::font_texture_channels>>(distanceFinder.distance(std::bit_cast<msdfgen::Point2>(p)));
                    float* const bitmap_begin = &bitmap[texture_map::font_texture_channels * (texture_map::font_texture_length_pixels * (texture_map::font_texture_length_pixels - y - 1) + x)];
                    for(std::size_t channel = 0; channel < texture_map::font_texture_channels; ++channel)
                        bitmap_begin[channel] = static_cast<float>(distance[channel] / texture_map::font_texture_distance_range + .5);
                }
            }

            msdfErrorCorrection(msdfgen::BitmapRef<float, 4>{bitmap.data(), texture_map::font_texture_length_pixels, texture_map::font_texture_length_pixels}, shape, msdfgen::Projection(texture_map::font_texture_glyph_scale, msdfgen::Vector2(msdf_padding.x(), msdf_padding.y())), msdfgen::Range(texture_map::font_texture_distance_range));

            for(std::size_t i = 0; i < texture_map::font_texture_size_bytes; ++i)
                glyphs[glyph_id][i] = static_cast<std::byte>(msdfgen::pixelFloatToByte(bitmap[i]));
        }
        if(ret != errc::unknown) return ret;


        font_data.glyph_bounds = std::move(glyph_bounds);
        font_data.glyph_padding = std::move(glyph_padding);
        std::vector<std::span<const std::byte>> glyph_spans(glyphs.size());
        for(std::size_t i = 0; i < glyphs.size(); ++i)
            glyph_spans[i] = std::span{reinterpret_cast<std::byte const*>(glyphs[i].data()), glyphs[i].size()};


        return create_texture(emplace_result.first, std::span{glyph_spans}, {texture_map::font_texture_length_pixels, texture_map::font_texture_length_pixels}, VK_FORMAT_R8G8B8A8_UNORM);
    }


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    result<texture_idx_t> renderable_tuple<FiF, std::tuple<Ts...>>::create_texture(typename texture_map::iterator tex_iter, std::span<std::span<const std::byte>> textures_as_bytes, extent2 texture_size, VkFormat format) noexcept {
        texture& tex = tex_iter->second;
        RESULT_TRY_MOVE(tex, make<texture>(
            logi_device_ptr,
            texture_size.width(), texture_size.height(), format, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            textures_as_bytes.size()
        ));

        renderable_allocator allocator(logi_device_ptr, phys_device_ptr, copy_cmd_pool_ptr);

        const std::size_t total_buffer_size = std::accumulate(textures_as_bytes.begin(), textures_as_bytes.end(), 0, [](std::size_t sum, std::span<const std::byte> bytes){ return sum + bytes.size_bytes(); });
        RESULT_VERIFY_UNSCOPED(allocator.stage(total_buffer_size, textures_as_bytes), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        
        //Re-allocate the memory
        device_memory<std::dynamic_extent> old_mem = std::move(texture_mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
        RESULT_TRY_MOVE(texture_mem, make<device_memory<std::dynamic_extent>>(logi_device_ptr, phys_device_ptr, textures, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
         
        //Re-create all other textures
        RESULT_VERIFY(allocator.gpu_alloc_begin());
        std::vector<texture> new_texs;
        new_texs.resize(textures.size());
        RESULT_VERIFY(allocator.realloc(new_texs, textures, texture_mem, std::distance(textures.begin(), tex_iter)));
        RESULT_VERIFY(allocator.gpu_alloc_end());

        //Replace the old textures with the new ones
        allocator.move(textures, new_texs);
        
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

        RESULT_VERIFY(tex.initialize(logi_device_ptr, phys_device_ptr, format));
        return tex.index();
    }
}



namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::index_buffer() const noexcept requires renderable_constraints<T>::has_indices { 
        if constexpr (renderable_constraints<T>::has_fixed_indices) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::uniform_buffer() const noexcept requires renderable_constraints<T>::has_uniform { 
        return uniform_buff; 
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::vertex_buffer() const noexcept requires renderable_constraints<T>::has_vertices  { 
        if constexpr (renderable_constraints<T>::has_fixed_vertices) return static_data_buff; 
        else return data_buffs[renderable_index<T>];
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::instance_buffer() const noexcept requires renderable_constraints<T>::has_instance_data { 
        return data_buffs[renderable_index<T>]; 
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::attribute_buffer() const noexcept requires renderable_constraints<T>::has_attributes { 
        return attribute_buffs[renderable_index_with_attrib<T>]; 
    };

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_tuple<FiF, std::tuple<Ts...>>::texture_idx_buffer() const noexcept requires renderable_constraints<T>::has_textures { 
        return data_buffs[renderable_index<T>];
    };


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::index_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices) {
        return this->template renderable_data_of<T>().index_count(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::vertex_count(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices) {
        return this->template renderable_data_of<T>().vertex_count(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::instance_count() const noexcept { 
        return this->template renderable_data_of<T>().instance_count();
    }


    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::first_index(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_indices && renderable_constraints<T>::has_indices) {
        return this->template renderable_data_of<T>().first_index(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_tuple<FiF, std::tuple<Ts...>>::first_vertex(std::size_t i) const noexcept requires (!renderable_constraints<T>::has_fixed_vertices && renderable_constraints<T>::has_vertices) {
        return this->template renderable_data_of<T>().first_vertex(i);
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::vertex_buffer_offset() const noexcept requires (renderable_constraints<T>::has_vertices) {
        if constexpr (renderable_constraints<T>::has_fixed_vertices) return static_offsets<T>()[buffer_data_type::vertex];
        else return this->template renderable_data_of<T>().vertex_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::index_buffer_offset() const noexcept requires (renderable_constraints<T>::has_indices) {
        if constexpr (renderable_constraints<T>::has_fixed_indices) return static_offsets<T>()[buffer_data_type::index];
        else return this->template renderable_data_of<T>().index_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_tuple<FiF, std::tuple<Ts...>>::texture_idx_buffer_offset() const noexcept requires (renderable_constraints<T>::has_textures) {
        return this->template renderable_data_of<T>().texture_idx_buffer_offset();
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_tuple<FiF, std::tuple<Ts...>>::static_offsets() noexcept { 
        //TODO: cleanup
        std::array<buffer_bytes_t, sizeof...(Ts)> buff_offsets;
        constexpr std::array<buffer_bytes_t,sizeof...(Ts)> static_sizes = {
            (buffer_bytes_t{
                renderable_data<Ts, FiF>::static_index_data_bytes.size(),
                renderable_data<Ts, FiF>::uniform_data_size,
                renderable_data<Ts, FiF>::static_vertex_data_bytes.size(),
                renderable_data<Ts, FiF>::instance_data_size,
                renderable_data<Ts, FiF>::attribute_data_size
            })...
        };
        
        std::exclusive_scan(static_sizes.cbegin(), static_sizes.cend(), buff_offsets.begin(), buffer_bytes_t{},
            [](buffer_bytes_t acc, const buffer_bytes_t& prev_size){ return buffer_bytes_t{
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                acc[buffer_data_type::uniform] + prev_size[buffer_data_type::uniform],
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                0,
                acc[buffer_data_type::attribute] + prev_size[buffer_data_type::attribute],
            };});
        for(std::size_t i = 0; i < sizeof...(Ts); ++i)
            buff_offsets[i][buffer_data_type::vertex] += static_sizes[i][buffer_data_type::index];
        return buff_offsets[renderable_index<T>];
    }
}


namespace d2d::vk {
    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename renderable_properties<T>::uniform_type, FiF> renderable_tuple<FiF, std::tuple<Ts...>>::uniform_map() const noexcept requires (renderable_constraints<T>::has_uniform) { 
        return std::span<typename renderable_properties<T>::uniform_type, FiF>{reinterpret_cast<typename renderable_properties<T>::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + static_offsets<T>()[buffer_data_type::uniform]), FiF}; 
    }

    template<std::size_t FiF, ::d2d::impl::directly_renderable... Ts> //requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<std::byte, 0> renderable_tuple<FiF, std::tuple<Ts...>>::uniform_map() const noexcept requires (!renderable_constraints<T>::has_uniform) { 
        return {}; 
    }
}