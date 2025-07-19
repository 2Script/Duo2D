#pragma once
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/graphics/ui/text.hpp"
#include <cstdlib>
#include <harfbuzz/hb.h>
#include <memory>
#include <string_view>

namespace d2d {
    inline text::text(std::size_t reserve) noexcept : 
        dynamic_renderable_container<text, glyph>(reserve, make_hybrid_for_overwrite<glyph>()), font_data_map_ptr{}, content_buffer(hb_buffer_create()), size_pixels{}, content{}, font_key(), glyph_count{}, starting_pos{} {}

    inline text::text(std::string_view contents, pt2f pos, font const& f, std::size_t font_size, true_color color, std::size_t extra_reserve) noexcept :
        dynamic_renderable_container<text, glyph>(contents.size() + extra_reserve, make_hybrid_for_overwrite<glyph>()), font_data_map_ptr{}, content_buffer(hb_buffer_create()), size_pixels{}, content{}, font_key(), glyph_count{}, starting_pos{} {
        emplace(contents, pos, f, font_size, color, extra_reserve);
    }
}

namespace d2d {
    inline text& text::emplace(std::string_view contents, pt2f pos, font const& f, std::size_t font_size, true_color color, std::size_t extra_reserve) noexcept {
        set_content(contents, pos, f, font_size, color);
        update_glyph_count();
        if(glyph_count > size())
            if(auto r = this->resize(glyph_count + extra_reserve, renderable<glyph>{{}, {f}, {}}, 0, font_size, pt2f{}, color); !r.has_value()) [[unlikely]]
                std::abort(); //TODO?: maybe change return type to a result and return the error code instead? (this function's signature will be inconsistent with std emplace functions in that case however)
        update_glyph_positions();
        return *this;
    }

    //TODO?: maybe use a different name for this function in order to not be different than std try_emplace function signatures
    inline std::pair<text&, bool> text::try_emplace(std::string_view contents, pt2f pos, font const& f, std::size_t font_size, true_color color) noexcept {
        set_content(contents, pos, f, font_size, color);
        update_glyph_count();
        if(glyph_count > size()) return {*this, false};

        update_glyph_positions();
        return {*this, true};
    }
}


namespace d2d {
    constexpr void text::set_content(std::string_view contents, pt2f pos, font const& f, std::size_t font_size, true_color color) noexcept {
        size_pixels = font_size;
        content = contents;
        font_key = f.key();
        starting_pos = pos;

        for(std::size_t i = 0; i < size(); ++i) {
            (*this)[i]->glyph_idx = 0;
            (*this)[i]->size = font_size;
            (*this)[i]->color = color;
            (*this)[i]->_fonts = {f}; //TODO make it not have to copy a string each time (i.e. separate fonts from glyphs) AND make it so fonts can be set without apply_changes
        }
    }

    
    inline void text::update_glyph_count() noexcept {
        if(font_data_map_ptr.expired()) return;
        std::shared_ptr<impl::font_data_map> font_data_map_inst = font_data_map_ptr.lock();
        if(!font_data_map_inst) return;
        auto find_font_data = font_data_map_inst->find(font_key);
        if(find_font_data == font_data_map_inst->cend()) return;
        impl::font_data const& font_data = find_font_data->second;

        //const std::size_t emulated_size_pixels = size_pixels + pixels_per_em / 2;
        hb_buffer_reset(content_buffer.get());
        hb_buffer_add_utf8(content_buffer.get(), content.data(), content.size(), 0, content.size());
        hb_buffer_guess_segment_properties(content_buffer.get());
        hb_font_set_scale(font_data.font_ptr.get(), size_pixels * scale_factor_pixels, size_pixels * scale_factor_pixels);
        hb_shape(font_data.font_ptr.get(), content_buffer.get(), nullptr, 0);
        
        glyph_count = hb_buffer_get_length(content_buffer.get());
    }


    //TODO: multithread for each line
    inline void text::update_glyph_positions() noexcept {
        if(glyph_count == 0) return;
        
        if(font_data_map_ptr.expired()) return;
        std::shared_ptr<impl::font_data_map> font_data_map_inst = font_data_map_ptr.lock();
        if(!font_data_map_inst) return;
        auto find_font_data = font_data_map_inst->find(font_key);
        if(find_font_data == font_data_map_inst->cend()) return;
        impl::font_data const& font_data = find_font_data->second;

        std::span<hb_glyph_info_t>     glyph_info = {hb_buffer_get_glyph_infos(content_buffer.get(), nullptr), glyph_count};
        std::span<hb_glyph_position_t> glyph_pos  = {hb_buffer_get_glyph_positions(content_buffer.get(), nullptr), glyph_count};

        const float units_per_em = hb_face_get_upem(font_data.face_ptr.get());
        const float font_units_per_pixel = units_per_em / pixels_per_em; 

        pt2f current_pos = starting_pos;
        for(std::size_t i = 0; i < glyph_count; ++i) {
            hb_codepoint_t glyph_idx = glyph_info[i].codepoint;
            (*this)[i]->glyph_idx = glyph_idx;
            
            pt2f offset = pt2i{glyph_pos[i].x_offset, glyph_pos[i].y_offset} / font_units_per_pixel;
            pt2f padding = (font_data.glyph_padding[glyph_idx].with_inverted_axis(axis::y) * size_pixels);
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
    constexpr result<void> text::after_changes_applied(basic_window<Ts...> const& win) noexcept {
        dynamic_renderable_container<text, glyph>::after_changes_applied(win);
        //TODO move to on_window_insert after fonts have been separated from glyphs
        bool changes_queued = font_data_map_ptr.expired();
        font_data_map_ptr = win.font_data_map();
        if(changes_queued) {
            update_glyph_count();
            update_glyph_positions();
        }
        return {}; 
    }
}
