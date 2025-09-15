#pragma once
#include <atomic>

namespace d2d {
    template<typename T>
    struct moveable_atomic : std::atomic<T> {
        constexpr moveable_atomic() noexcept = default;
        constexpr moveable_atomic(T value) noexcept : std::atomic<T>(value) {} 

        constexpr moveable_atomic(moveable_atomic&& other) noexcept : std::atomic<T>(other.exchange(T{})) {}
        constexpr moveable_atomic& operator=(moveable_atomic&& other) noexcept { this->store(other.exchange(T{}), std::memory_order::relaxed); return *this; }

        moveable_atomic(moveable_atomic const& other) = delete;
        moveable_atomic& operator=(moveable_atomic const& other) = delete;
    };
}