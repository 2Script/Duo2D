#pragma once
#include <optional>

#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/arith/point.hpp"


namespace d2d::input {
    using mouse_aux_t = std::optional<d2d::pt2d>;
}

namespace d2d::input{
    using generic_event_function = void(void*, combination, bool, categorized_event_t, mouse_aux_t, void*);
    using text_event_function = void(void*, unsigned int);
}