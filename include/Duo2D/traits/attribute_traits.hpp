#pragma once
#include <span>
#include <tuple>
#include <utility>
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include "Duo2D/traits/aggregate_traits.hpp"
#include "Duo2D/traits/shader_input_traits.hpp"


namespace d2d::impl {
    template<typename T> struct attribute_traits;
    template<typename... Ts> 
    struct attribute_traits<std::tuple<attribute<Ts>&...>>  {
        constexpr static std::size_t total_size = (sizeof(Ts) + ...);
        constexpr static std::array<std::size_t, sizeof...(Ts)> sizes{ sizeof(Ts)... };

        using tuple_type = decltype(std::tuple_cat(std::declval<as_tuple_t<Ts>>()...));
        constexpr static std::size_t total_member_count = std::tuple_size_v<tuple_type>;
        //using tuple_type = decltype(std::tuple_cat(to_tuple(size_constant<member_count<Ts>()>{}, std::declval<Ts>())...));
        //constexpr static std::size_t total_member_count = (member_count<Ts>() + ...);
    };
}

namespace d2d::impl {
    template<typename T>
    struct attributes_span_type { using type = std::tuple<>; };

    template<typename T> requires renderable_constraints<T>::has_attributes
    struct attributes_span_type<T> { using type = std::tuple<std::span<std::byte>>; };

    template<typename... Ts>
    using attributes_tuple = decltype(std::tuple_cat(
        std::declval<typename attributes_span_type<Ts>::type>()...
    )); 
}

namespace d2d {
    template<typename... Ts>
    struct make_attribute_types { using type = std::tuple<attribute<Ts>&...>; };

    template<typename... Ts>
    using make_attribute_types_t = typename make_attribute_types<Ts...>::type;
}