#pragma once
#include <Duo2D/core/basic_window.hpp>

namespace d2d::impl {
    struct renderable_event_callbacks {
        template<typename... Ts, typename UniformT, std::size_t N>
        constexpr static void on_swap_chain_update(basic_window<Ts...> const&, std::span<UniformT, N>) noexcept {}

    };
}