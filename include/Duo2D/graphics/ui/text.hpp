#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/graphics/core/renderable_container.hpp"
#include "Duo2D/graphics/prim/glyph.hpp"

namespace d2d {
    //Use variable_renderable_container
    class text : public dynamic_renderable_container<glyph> {
        using glyph_count_t = unsigned int;
    public:
        using base_type = dynamic_renderable_container<glyph>;
        constexpr static std::size_t default_reserved_size = 0x40;

    public:
        inline text(std::size_t reserve = default_reserved_size) noexcept;
        inline text(std::string_view contents, pt2f pos, font const& f, font_size_t font_size = 24, true_color color = 0x00'00'00'FF, std::size_t extra_reserve = 0) noexcept;

    public:
        inline text(text&& other) noexcept;
        inline text& operator=(text&& other) noexcept;
        text(text const& other) noexcept = delete;
        text& operator=(text const& other) noexcept = delete;
        ~text() noexcept = default;

    public:
        inline text& emplace(std::string_view contents, pt2f pos, font const& f, font_size_t font_size = 24, true_color color = 0x00'00'00'FF, std::size_t extra_reserve = 0) noexcept;
        inline std::pair<text&, bool> try_emplace(std::string_view contents, pt2f pos, font const& f, font_size_t font_size = 24, true_color color = 0x00'00'00'FF) noexcept;
        
    
    private:
        constexpr void set_content(std::string_view contents, pt2f pos, font const& f, font_size_t font_size, true_color color) noexcept;
        inline std::pair<glyph_count_t, impl::font_data const*> shape_glyphs() noexcept;
        inline void update_glyph_positions(text::glyph_count_t glyph_count, impl::font_data const* font_data_ptr) noexcept;


    public:
        template<typename U, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>& win, std::uint64_t insertion_key) noexcept;


    private:
        std::weak_ptr<impl::font_data_map> font_data_map_ptr;
        std::unique_ptr<hb_buffer_t, generic_functor<hb_buffer_destroy>> content_buffer;
        std::string content;
        font font_key;
        pt2f starting_pos;
        font_size_t size_pixels;
    private:
        constexpr static std::size_t pixels_per_em = 16; //(assumed to be) the value msdfgen uses when creating font textures
        constexpr static float padding = vk::texture_map::font_texture_padding_em * pixels_per_em;
        constexpr static pt2f padding_pixels{-padding, padding};
        constexpr static std::size_t scale_factor_pixels = vk::texture_map::font_texture_length_pixels * 2;
    };
}


#include "Duo2D/graphics/ui/text.inl"