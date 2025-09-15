#pragma once
#include <cstddef>
#include <limits>
#include <bitset>
#include <algorithm>

#include "Duo2D/input/event_int.hpp"


namespace d2d::input {
    //TODO: reserve 0 for no event
    namespace event {
    enum : event_id_t {
        //TEMP
        mouse_move,
        mouse_press,
        mouse_release,
        next_focus,

        num_default_event_ids,
    };
    }
}
