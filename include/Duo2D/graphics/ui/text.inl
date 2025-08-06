#pragma once
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/graphics/ui/text.hpp"
#include <cstdlib>
#include <harfbuzz/hb.h>
#include <memory>
#include <result.hpp>
#include <string_view>

namespace d2d {
    inline text::text(std::size_t reserve) noexcept : 
        base_type{dynamic_renderable_container<text, glyph>(reserve, make_hybrid<glyph>(renderable<glyph>{}, 0, 0, pt2f{}, true_color{}))}, font_data_map_ptr{}, content_buffer(hb_buffer_create()), content{}, font_key(), starting_pos{}, size_pixels{} {}

    inline text::text(std::string_view contents, pt2f pos, font const& f, font_size_t font_size, true_color color, std::size_t extra_reserve) noexcept :
        base_type{dynamic_renderable_container<text, glyph>(contents.size() + extra_reserve, make_hybrid_for_overwrite<glyph>())}, font_data_map_ptr{}, content_buffer(hb_buffer_create()), content{}, font_key(), starting_pos{}, size_pixels{} {
        emplace(contents, pos, f, font_size, color, extra_reserve);
    }
}

namespace d2d {
    inline text::text(text&& other) noexcept :
        base_type(std::move(other)), 
        content_buffer(std::move(other.content_buffer)),
        content(std::move(other.content)), 
        font_key(std::move(other.font_key)),
        starting_pos(other.starting_pos), 
        size_pixels(other.size_pixels) {        
        for(std::size_t i = 0; i < size(); ++i)
            (*this)[i]->_fonts = {static_cast<font_view>(font_key)};
    } 

    inline text& text::operator=(text&& other) noexcept {
        base_type::operator=(std::move(other));
        content_buffer = std::move(other.content_buffer);
        content = std::move(other.content);
        font_key = std::move(other.font_key);
        starting_pos = other.starting_pos;
        size_pixels = other.size_pixels;
        for(std::size_t i = 0; i < size(); ++i)
            (*this)[i]->_fonts = {static_cast<font_view>(font_key)};
        return *this;
    }
}

namespace d2d {
    inline text& text::emplace(std::string_view contents, pt2f pos, font const& f, font_size_t font_size, true_color color, std::size_t extra_reserve) noexcept {
        set_content(contents, pos, f, font_size, color);
        auto [glyph_count, font_data_ptr] = shape_glyphs();
        if(glyph_count + extra_reserve > size())
            if(auto r = resize(glyph_count + extra_reserve, renderable<glyph>{{}, {f}, {}}, 0, font_size, pt2f{}, color); !r.has_value()) [[unlikely]]
                std::abort(); //TODO?: maybe change return type to a result and return the error code instead? (this function's signature will be inconsistent with std emplace functions in that case however)
        update_glyph_positions(glyph_count, font_data_ptr);
        return *this;
    }

    //TODO?: maybe use a different name for this function so that it's consistent std try_emplace function signatures
    inline std::pair<text&, bool> text::try_emplace(std::string_view contents, pt2f pos, font const& f, font_size_t font_size, true_color color) noexcept {
        set_content(contents, pos, f, font_size, color);
        auto [glyph_count, font_data_ptr] = shape_glyphs();
        if(glyph_count > size()) return {*this, false};

        update_glyph_positions(glyph_count, font_data_ptr);
        return {*this, true};
    }
}


namespace d2d {
    constexpr void text::set_content(std::string_view contents, pt2f pos, font const& f, font_size_t font_size, true_color color) noexcept {
        size_pixels = font_size;
        content = contents;
        font_key = f;
        starting_pos = pos;

        for(std::size_t i = 0; i < size(); ++i) {
            (*this)[i]->glyph_idx = 0;
            (*this)[i]->size = font_size;
            (*this)[i]->color = color;
            (*this)[i]->_fonts = {static_cast<font_view>(font_key)};
        }
    }

    
    inline std::pair<text::glyph_count_t, impl::font_data const*> text::shape_glyphs() noexcept {
        if(font_data_map_ptr.expired()) return {0, nullptr};
        std::shared_ptr<impl::font_data_map> font_data_map_inst = font_data_map_ptr.lock();
        if(!font_data_map_inst) return {0, nullptr};
        auto find_font_data = font_data_map_inst->find(font_key);
        if(find_font_data == font_data_map_inst->cend()) return {0, nullptr};
        impl::font_data const* font_data_ptr = &(find_font_data->second);

        //const std::size_t emulated_size_pixels = size_pixels + pixels_per_em / 2;
        hb_buffer_reset(content_buffer.get());
        hb_buffer_add_utf8(content_buffer.get(), content.data(), content.size(), 0, content.size());
        hb_buffer_guess_segment_properties(content_buffer.get());
        hb_font_set_scale(font_data_ptr->font_ptr.get(), size_pixels * scale_factor_pixels, size_pixels * scale_factor_pixels);
        hb_shape(font_data_ptr->font_ptr.get(), content_buffer.get(), nullptr, 0);
        
        return std::make_pair(hb_buffer_get_length(content_buffer.get()), font_data_ptr);
    }


    //TODO: multithread for each line
    inline void text::update_glyph_positions(text::glyph_count_t glyph_count, impl::font_data const* font_data_ptr) noexcept {
        if(glyph_count == 0 || !font_data_ptr) return;

        std::span<hb_glyph_info_t>     glyph_info = {hb_buffer_get_glyph_infos(content_buffer.get(), nullptr), glyph_count};
        std::span<hb_glyph_position_t> glyph_pos  = {hb_buffer_get_glyph_positions(content_buffer.get(), nullptr), glyph_count};

        const float units_per_em = hb_face_get_upem(font_data_ptr->face_ptr.get());
        const float font_units_per_pixel = units_per_em / pixels_per_em; 

        pt2f current_pos = starting_pos;
        for(std::size_t i = 0; i < glyph_count; ++i) {
            hb_codepoint_t glyph_idx = glyph_info[i].codepoint;
            (*this)[i]->glyph_idx = glyph_idx;
            
            pt2f offset = pt2i{glyph_pos[i].x_offset, glyph_pos[i].y_offset} / font_units_per_pixel;
            pt2f padding = (font_data_ptr->glyph_padding[glyph_idx].with_inverted_axis(axis::y) * size_pixels);
            (*this)[i]->pos = current_pos - padding + offset;
            pt2f advance = pt2i{glyph_pos[i].x_advance, glyph_pos[i].y_advance} / font_units_per_pixel;
            current_pos += advance;
        }
        for(std::size_t i = glyph_count; i < size(); ++i)
            (*this)[i]->glyph_idx = 0;
    }
}



namespace d2d {
    template<typename... Ts>
    constexpr void text::on_window_insert(basic_window<Ts...>& win, std::string_view insertion_key) noexcept {
        base_type::on_window_insert(win, insertion_key);

        bool changes_queued = font_data_map_ptr.expired();
        font_data_map_ptr = win.font_data_map();

        if(!changes_queued) return;
        auto [glyph_count, font_data_ptr] = shape_glyphs();
        update_glyph_positions(glyph_count, font_data_ptr);
    }
}
