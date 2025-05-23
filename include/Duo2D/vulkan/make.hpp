#pragma once
#include <utility>
#include "Duo2D/error.hpp"

namespace d2d::impl {
    template<typename T, typename... Args>
    concept vulkan_ptr_like = requires { {T::create(std::declval<Args>()...)} noexcept -> std::same_as<result<T>>; };
}


namespace d2d {
    template<typename T, typename... Args> requires impl::vulkan_ptr_like<T, Args...>
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

#define __D2D_TRY_MAKE(lhs, rhs, var_name) \
auto var_name = rhs; \
if(!var_name.has_value()) return var_name.error(); \
lhs = *std::move(var_name);