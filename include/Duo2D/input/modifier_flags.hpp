#pragma once
#include <cstdint>


namespace d2d::input {
    using modifier_flags_t = std::uint8_t;
}

namespace d2d::input {
    namespace modifier_flags {
    enum : modifier_flags_t {
        no_modifiers_allowed = 1 << 0,
    };
    }
}