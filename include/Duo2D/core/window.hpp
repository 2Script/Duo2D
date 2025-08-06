#pragma once
#include "Duo2D/core/basic_window.hpp"

#include "Duo2D/graphics/prim/debug_rect.hpp"
#include "Duo2D/graphics/prim/glyph.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/graphics/ui/text.hpp"
#include "Duo2D/graphics/ui/progress_bar.hpp"

namespace d2d {
    template<typename... Ts>
    using extended_window = basic_window<text_tuple, progress_bar, text, styled_rect, debug_rect, clone_rect, glyph, Ts...>;

    using window = extended_window<>;
}
