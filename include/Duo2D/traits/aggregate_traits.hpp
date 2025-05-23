#pragma once
#include <type_traits>
#include "Duo2D/traits/shader_input_traits.hpp"

namespace d2d::impl {
    template<class T> concept aggregate = std::is_aggregate_v<T>;
    template<class T> concept arithmetic = std::is_arithmetic_v<T>;

    struct any_t { template<class T> operator T() {} };

    template<aggregate T, typename... M>
    consteval std::size_t member_count(M&&... members)  {
        if constexpr (requires (T t){ T{members...}; t = {members...}; })
            return member_count<T>(std::forward<M>(members)..., any_t{});
        return sizeof...(members) - 1;
    }
    template<arithmetic T>
    consteval std::size_t member_count()  { return 1; }
    
    template<std::size_t N, typename T>
    constexpr decltype(auto) to_tuple(size_constant<N>, T const&) { static_assert(N != N, "Too many aggregate members (not yet supported)"); }

    template<arithmetic T> constexpr decltype(auto) to_tuple(size_constant<1>, T const& t) { return std::make_tuple(t); }

    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<0>, T const&) { return std::tuple(); }
    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<1>, T const& agg) { auto& [m1] = agg; return std::tuple(m1); }
    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<2>, T const& agg) { auto& [m1, m2] = agg; return std::tuple(m1, m2); }
    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<3>, T const& agg) { auto& [m1, m2, m3] = agg; return std::tuple(m1, m2, m3); }
    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<4>, T const& agg) { auto& [m1, m2, m3, m4] = agg; return std::tuple(m1, m2, m3, m4); }
    template<aggregate T> constexpr decltype(auto) to_tuple(size_constant<5>, T const& agg) { auto& [m1, m2, m3, m4, m5] = agg; return std::tuple(m1, m2, m3, m4, m5); }

    template<typename T>
    struct as_tuple { using type = decltype(to_tuple(size_constant<member_count<T>()>{}, std::declval<T>())); };
    template<typename... Ts>
    struct as_tuple<std::tuple<Ts...>> { using type = std::tuple<Ts...>; };

    template<typename T> using as_tuple_t = typename as_tuple<T>::type;
};
