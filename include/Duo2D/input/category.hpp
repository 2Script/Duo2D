#pragma once
#include <cstddef>
#include <limits>
#include <bitset>
#include <algorithm>

#include "Duo2D/input/event_int.hpp"


namespace d2d::input {
    namespace category {
    enum : category_id_t {
        system,
        ui,

        num_default_categories,
    };
    }
}


namespace d2d::input::impl {
    constexpr std::size_t num_category_values = (std::numeric_limits<category_id_t>::max() + static_cast<std::size_t>(1));
    constexpr std::size_t raw_num_categories = std::min(num_category_values, static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits));
}

namespace d2d::input {
    //TODO make this able to be set by the user at compile time
    constexpr std::size_t num_categories = impl::raw_num_categories;
    //For better alignment - makes space in event_set (by slightly lowing max number of categories) so that the category flags can fit within an event_set with a power of 2 alignment/size (e.g. 256 bytes)
    //constexpr std::size_t num_categories = impl::raw_num_categories - sizeof(std::bitset<impl::raw_num_categories>) / sizeof(event_id_t);
    constexpr std::size_t max_category_id = num_categories - 1;
    using category_flags_t = std::bitset<num_categories>;
}