#pragma once
#include <cstddef>
#include <array>
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/vector.hpp"


namespace d2d {
    struct buffer_data_type{ enum {
        index, uniform, vertex, instance,
        count
    };};

    using buffer_bytes_t = vector<buffer_data_type::count, std::size_t>;
}

namespace d2d::impl {
    struct type_filter { enum idx {
        none, has_index, has_uniform, has_instance, has_push_const, has_attrib, has_storage,
        count
    };};

    template <typename T, typename U, typename... Ts>
    struct type_index {
        constexpr static std::array<std::size_t, type_filter::count> value = {
            1                                         + type_index<T, Ts...>::value[type_filter::none],
            (!U::instanced && impl::has_indices_v<U>) + type_index<T, Ts...>::value[type_filter::has_index],
            (impl::has_uniform_v<U>)                  + type_index<T, Ts...>::value[type_filter::has_uniform],
            (U::instanced)                            + type_index<T, Ts...>::value[type_filter::has_instance],
            (impl::has_push_constants_v<U>)           + type_index<T, Ts...>::value[type_filter::has_push_const],
            (impl::has_attributes_v<U>)               + type_index<T, Ts...>::value[type_filter::has_attrib],
            (impl::has_storage_v<U>)                  + type_index<T, Ts...>::value[type_filter::has_storage],
        };
    };
    
    template <typename T, typename... Ts>
    struct type_index<T, T, Ts...> {
        constexpr static std::array<std::size_t, type_filter::count> value = {};
    };

    template<typename... Ts>
    struct type_count {
        constexpr static std::array<std::size_t, type_filter::count> value = {
            sizeof...(Ts),
            ((!Ts::instanced && impl::has_indices_v<Ts>) + ...),
            ((impl::has_uniform_v<Ts>)                   + ...),
            ((Ts::instanced)                             + ...),
            ((impl::has_push_constants_v<Ts>)            + ...),
            ((impl::has_attributes_v<Ts>)                + ...),
            ((impl::has_storage_v<Ts>)                   + ...),
        };
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