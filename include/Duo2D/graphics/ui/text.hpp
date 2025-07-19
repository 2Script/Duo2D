#pragma once
#include <cstddef>
#include <memory>
#include <string_view>
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/graphics/core/renderable_container.hpp"
#include "Duo2D/graphics/prim/glyph.hpp"

namespace d2d {
    //Use variable_renderable_container
    class text : public dynamic_renderable_container<text, glyph>{
    public:
        constexpr static std::size_t default_reserved_size = 0x40;

    public:
        inline text(std::size_t reserve = default_reserved_size) noexcept;
        inline text(std::string_view contents, pt2f pos, font const& f, std::size_t font_size = 24, true_color color = 0x00'00'00'FF, std::size_t extra_reserve = 0) noexcept;

    public:
        inline text& emplace(std::string_view contents, pt2f pos, font const& f, std::size_t font_size = 24, true_color color = 0x00'00'00'FF, std::size_t extra_reserve = 0) noexcept;
        inline std::pair<text&, bool> try_emplace(std::string_view contents, pt2f pos, font const& f, std::size_t font_size = 24, true_color color = 0x00'00'00'FF) noexcept;
        
    
    private:
        constexpr void set_content(std::string_view contents, pt2f pos, font const& f, std::size_t font_size, true_color color) noexcept;
        inline    void update_glyph_count() noexcept;
        inline    void update_glyph_positions() noexcept;


    public:
        template<typename... Ts>
        constexpr result<void> after_changes_applied(basic_window<Ts...> const& win) noexcept;
        

    private:
        std::weak_ptr<impl::font_data_map> font_data_map_ptr;
        std::unique_ptr<hb_buffer_t, generic_functor<hb_buffer_destroy>> content_buffer;
        std::size_t size_pixels;
        std::string_view content;
        std::string font_key;
        unsigned int glyph_count;
        pt2f starting_pos;
    private:
        constexpr static std::size_t pixels_per_em = 16; //(assumed to be) the value msdfgen uses when creating font textures
        constexpr static float padding = vk::texture_map::font_texture_padding_em * pixels_per_em;
        constexpr static pt2f padding_pixels{-padding, padding};
        constexpr static std::size_t scale_factor_pixels = vk::texture_map::font_texture_length_pixels * 2;
    };
}


#include "Duo2D/graphics/ui/text.inl"