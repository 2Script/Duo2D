#pragma once
#include "Duo2D/core/resource_table.hpp"

namespace d2d {
    template<typename TimelineT, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    struct window;
}
