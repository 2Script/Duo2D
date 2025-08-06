#pragma once
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace d2d::impl {
    template<typename T, typename U = std::remove_cvref_t<T>>
    concept renderable_container_like = requires (T t, std::size_t i) {
        typename U::value_type;
        typename U::renderable_type;
        {t.size()} noexcept;
        {t[i]} noexcept;
    };
}

namespace d2d::impl {
    template<typename T>
    concept renderable_container_tuple_like = renderable_container_like<typename std::tuple_element<0, typename std::remove_cvref_t<T>::tuple_type>::type>;
}