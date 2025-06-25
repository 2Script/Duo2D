#pragma once
#include <utility>

namespace d2d {
    template<auto Func>
    struct generic_functor {
        template<typename... Args>
        constexpr decltype(auto) operator()(Args&&... args) { 
            return Func(std::forward<Args>(args)...); 
        }
    };
}