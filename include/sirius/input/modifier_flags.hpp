#pragma once
#include <cstdint>


namespace acma::input {
    using modifier_flags_t = std::uint8_t;
}

namespace acma::input {
    namespace modifier_flags {
    enum : modifier_flags_t {
        no_modifiers_allowed = 1 << 0,
    };
    }
}