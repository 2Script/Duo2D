#pragma once
#include <type_traits>
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

/*
namespace d2d {
    template<typename C, auto MemFunc>
    struct stateful_generic_member_functor {
        C* _ptr; 

        template<typename... Args>
        constexpr decltype(auto) operator()(Args&&... args) { 
            return MemFunc(_ptr, std::forward<Args>(args)...); 
        }
    };
}
*/