#pragma once
#include <array>

#include "Duo2D/input/event_int.hpp"
#include "Duo2D/input/category.hpp"


namespace d2d::input {
    struct event_set {
        std::array<event_id_t, num_categories> event_ids{};
        category_flags_t applicable_categories{};
    };
}