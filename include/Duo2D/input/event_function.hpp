#pragma once
#include <variant>

#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/arith/point.hpp"


namespace d2d::input {
    using mouse_aux_t = std::variant<std::monostate, pt2d, double>;
}

namespace d2d::input{
    using generic_event_function = void(void*, combination, bool, event_t, mouse_aux_t, void*);
    using text_event_function = void(void*, unsigned int);
}