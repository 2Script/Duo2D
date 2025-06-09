#pragma once
#include <cstddef>

namespace d2d::impl {
    template <bool B, std::size_t V>
    struct conditional_size {
        constexpr static std::size_t value = V;
    };

    template <std::size_t V>
    struct conditional_size<false, V> {
        constexpr static std::size_t value = 0;
    };

    template <bool B, std::size_t V>
    constexpr static std::size_t conditional_size_v = conditional_size<B, V>::value;
}