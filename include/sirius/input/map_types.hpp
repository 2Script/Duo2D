#pragma once
#include <unordered_map>

#include "sirius/input/combination.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/event_int.hpp"
#include "sirius/input/event_set.hpp"

namespace acma::input {
    using binding_map = std::unordered_map<combination, event_set>;
    using event_fns_map = std::unordered_map<categorized_event_t, std::function<generic_event_function>>;
}