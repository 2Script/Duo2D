#pragma once
#include "Duo2D/graphics/ui/progress_bar.hpp"

#include <cstddef>

#include "Duo2D/arith/rect.hpp"
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"

namespace d2d {
    progress_bar::progress_bar(std::size_t text_reserve) noexcept :
        base_type(make_hybrid<renderable_container<d2d::styled_rect, 2>>(make_hybrid<styled_rect>(), make_hybrid<styled_rect>()), make_hybrid<d2d::text>(text_reserve), make_hybrid<text_tuple>(), make_hybrid<styled_rect>()) {}

    progress_bar::progress_bar(pt2f pos, size2f bar_size, font const& text_font, true_color border_color, true_color bar_color, true_color text_color, font_size_t text_font_size, std::size_t text_reserve) noexcept : base_type(
        make_hybrid<renderable_container<d2d::styled_rect, 2>>(
            make_hybrid<styled_rect>(rect<float>{pos, bar_size}, border_color),
            make_hybrid<styled_rect>(rect<float>{pos + (bar_size * .05f), bar_size * .90f}, bar_color)
        ),
        make_hybrid<d2d::text>("<progress bar text>", pt2f{pos.x() + bar_size.width()/2, pos.y() + bar_size.height()}, text_font, text_font_size, text_color, text_reserve),
        make_hybrid<text_tuple>(), 
        make_hybrid<styled_rect>()
    ) {}
}