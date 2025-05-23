#pragma once
#include <cstddef>
#include <array>
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/vector.hpp"


namespace d2d {
    struct buffer_data_type{ enum {
        index, uniform, vertex, instance, attribute,
        count
    };};

    using buffer_bytes_t = vector<buffer_data_type::count, std::size_t>;
}

namespace d2d::impl {
    struct type_filter { enum idx {
        none, not_instanced, has_index, has_uniform, has_push_const, has_attrib, has_storage,
        count
    };};

    template <typename T, typename U, typename... Ts>
    struct type_index {
        constexpr static std::size_t value = 1 + type_index<T, Ts...>::value;
        constexpr static std::size_t with_attrib_value = (U::has_attributes) + type_index<T, Ts...>::with_attrib_value;
    };
    
    template <typename T, typename... Ts>
    struct type_index<T, T, Ts...> {
        constexpr static std::size_t value = 0;
        constexpr static std::size_t with_attrib_value = 0;
    };
}

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