#pragma once
#include <cstddef>
#include <type_traits>

namespace d2d {
    template<std::size_t V> 
    using size_constant = std::integral_constant<std::size_t, V>;
}