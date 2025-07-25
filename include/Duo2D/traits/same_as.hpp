#pragma once
#include <concepts>
#include <type_traits>

namespace d2d::impl {
    template<typename T, typename U>
    concept same_as = std::same_as<T, U>;

    template<typename T, typename U>
    concept when_decayed_same_as = std::same_as<std::decay_t<T>, U>;
}