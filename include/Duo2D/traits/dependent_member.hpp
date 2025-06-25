#pragma once
#include <cstddef>

namespace d2d::impl {
    template<typename T>
    struct dependent_member { 
        T data; 
    };

    template<> struct dependent_member<void> {};
}

namespace d2d::impl {
    template<typename T, auto DestroyFn>
    struct dependent_static_member { 
        inline static T data{};
    };

    template<auto DestroyFn> struct dependent_static_member<void, DestroyFn> {};
}