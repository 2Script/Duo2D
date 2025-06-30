#pragma once
#include <utility>
#include "Duo2D/core/error.hpp"

namespace d2d::impl {
    template<typename T, typename... Args>
    concept result_constructible = requires { {T::create(std::declval<Args>()...)} noexcept -> std::same_as<result<T>>; };
}


namespace d2d {
    template<typename T, typename... Args> requires impl::result_constructible<T, Args...>
    inline result<T> make(Args&&... args) noexcept {
        /*constexpr bool dependent = requires { T::dependent_handle; };
        if constexpr(dependent) {
            result<T> ret = T::create(std::forward<Args>(args)...);
            if(ret.has_value() && !ret->dependent_handle)
                return error::unknown;
        }
        else*/ return T::create(std::forward<Args>(args)...);
    } 
}
