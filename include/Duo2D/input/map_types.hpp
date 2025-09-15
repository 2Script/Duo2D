#pragma once
#include <unordered_map>

#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/input/event_set.hpp"

namespace d2d::input {
    using binding_map = std::unordered_map<combination, event_set>;
    using event_fns_map = std::unordered_map<categorized_event_t, std::function<generic_event_function>>;
}