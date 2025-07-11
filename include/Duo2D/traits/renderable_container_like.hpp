#pragma once
#include <cstddef>

namespace d2d::impl {
    template<typename T>
    concept renderable_container_like = requires (T t, std::size_t i) {
        typename T::value_type;
        typename T::element_type;
        {t.size()} noexcept;
        {t[i]} noexcept;
    };
}