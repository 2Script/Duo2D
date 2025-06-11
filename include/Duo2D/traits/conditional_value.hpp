#pragma once
#include <cstddef>

namespace d2d::impl {
    template <bool B, typename T, T TrueV, T FalseV>
    struct conditional_value {
        constexpr static T value = TrueV;
    };

    template <typename T, T TrueV, T FalseV>
    struct conditional_value<false, T, TrueV, FalseV> {
        constexpr static T value = FalseV;
    };

    template <bool B, typename T, T TrueV, T FalseV>
    constexpr static T conditional_value_v = conditional_value<B, T, TrueV, FalseV>::value;
}

namespace d2d::impl {
    template <bool B, std::size_t V>
    using conditional_size = conditional_value<B, std::size_t, V, 0>;
    template <bool B, std::size_t V>
    constexpr static std::size_t conditional_size_v = conditional_size<B, V>::value;
}