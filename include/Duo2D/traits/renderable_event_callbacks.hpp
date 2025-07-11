#pragma once
#include <span>
#include <cstddef>
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/basic_window.hpp"

namespace d2d::impl {
    struct renderable_event_callbacks {
    public:
        template<typename... Ts, typename UniformT, std::size_t N>
        constexpr static void on_swap_chain_update(basic_window<Ts...> const&, std::span<UniformT, N>) noexcept {}

    public:
        template<typename T, typename... Ts>
        constexpr void on_window_insert(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<T>) noexcept {}
        template<typename T, typename... Ts>
        constexpr void on_window_insert_child(basic_window<Ts...>&, typename basic_window<Ts...>::template iterator<T>, std::size_t) noexcept {}

    public:
        template<typename... Ts>
        constexpr result<void> before_changes_applied(basic_window<Ts...> const&) noexcept { return {}; }
        template<typename... Ts>
        constexpr result<void>  after_changes_applied(basic_window<Ts...> const&) noexcept { return {}; }
    };
}