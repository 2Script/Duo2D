#pragma once
#include <concepts>
#include <type_traits>

namespace d2d::impl {
    template<typename T, typename U>
    concept different_from = !std::same_as<T, U>;

    template<typename T, typename U>
    concept when_decayed_different_from = !std::same_as<std::decay_t<T>, U>;
}