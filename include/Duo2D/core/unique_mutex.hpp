#pragma once
#include <mutex>

namespace d2d {
    struct unique_mutex : std::mutex {
        constexpr unique_mutex() noexcept = default;

        //This is specific for our use case because we will never be using a mutex and moving it at the same time
        constexpr unique_mutex(unique_mutex&&) noexcept {}
        constexpr unique_mutex& operator=(unique_mutex&&) noexcept { return *this; }

        unique_mutex(unique_mutex const& other) = delete;
        unique_mutex& operator=(unique_mutex const& other) = delete;
    };
}