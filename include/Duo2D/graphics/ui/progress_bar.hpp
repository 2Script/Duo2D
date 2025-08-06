#pragma once
#include <cstddef>

#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable_container.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/graphics/ui/text.hpp"

namespace d2d {
    //only for testing
    class text_tuple : public renderable_container_tuple<text, text> {
    public:
        inline text_tuple() noexcept : renderable_container_tuple<text, text>(make_hybrid<text>(), make_hybrid<text>()) {};
    };
}

namespace d2d {
    //"fake" progress bar - not the final product, just used for testing
    class progress_bar : public renderable_container_tuple<renderable_container<d2d::styled_rect, 2>, text, text_tuple, styled_rect> {
    public:
        using base_type = renderable_container_tuple<renderable_container<d2d::styled_rect, 2>, text, text_tuple, styled_rect>;
    public:
        inline progress_bar(std::size_t text_reserve = 0) noexcept;
        inline progress_bar(pt2f pos, size2f bar_size, font const& text_font, true_color border_color, true_color bar_color, true_color text_color = 0x00'00'00'FF, font_size_t text_font_size = 24, std::size_t text_reserve = 0) noexcept;
        
    };
}

#include "Duo2D/graphics/ui/progress_bar.inl"