#pragma once
#include <thread>

#include <BS_thread_pool.hpp>


namespace d2d {
    inline BS::priority_thread_pool& thread_pool() {
        //leave 2 threads open, 1 for the main thread and 1 for a single application render thread.
        //This assumes there will only be 1 application at a time. If there are more, there may be performance penalties
        static BS::priority_thread_pool tp{std::thread::hardware_concurrency() - (std::thread::hardware_concurrency() > 2 ? 2 : 0)};
        return tp;
    }
}