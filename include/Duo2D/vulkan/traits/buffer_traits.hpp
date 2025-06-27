#pragma once
#include <cstddef>
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"


namespace d2d::vk {
    struct buffer_data_type{ enum {
        index, uniform, vertex, instance, attribute,
        count
    };};

    using buffer_bytes_t = vector<buffer_data_type::count, std::size_t>;
}

namespace d2d::vk::impl {
    template <typename T, typename U, typename... Ts>
    struct buffer_type_index {
        constexpr static std::size_t value = 1 + buffer_type_index<T, Ts...>::value;
        constexpr static std::size_t with_attrib_value = (renderable_constraints<U>::has_attributes) + buffer_type_index<T, Ts...>::with_attrib_value;
    };
    
    template <typename T, typename... Ts>
    struct buffer_type_index<T, T, Ts...> {
        constexpr static std::size_t value = 0;
        constexpr static std::size_t with_attrib_value = 0;
    };
}