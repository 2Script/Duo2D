#pragma once
#include <array>

#include "sirius/input/event_int.hpp"
#include "sirius/input/category.hpp"


namespace acma::input {
    struct event_set {
        std::array<event_id_t, num_categories> event_ids{};
        category_flags_t applicable_categories{};
    };
}