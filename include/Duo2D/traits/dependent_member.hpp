#pragma once
#include <cstddef>

namespace d2d::impl {
    template<typename T>
    struct dependent_member { 
        T data; 
    };

    template<> struct dependent_member<void> {};
}