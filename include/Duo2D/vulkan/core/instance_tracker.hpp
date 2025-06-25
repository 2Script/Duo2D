#pragma once
#include <cstddef>
#include <type_traits>
#include "Duo2D/core/error.hpp"
#include "Duo2D/traits/dependent_member.hpp"


namespace d2d::impl {
    template<typename T, auto DestroyFn> 
    class instance_tracker : public dependent_static_member<T, DestroyFn> {
        constexpr static bool has_data = !std::is_same_v<T, void>;
    public:
        constexpr instance_tracker() noexcept = default;

        template<typename CreateFnT, typename... Args>
        constexpr result<void> initialize(std::atomic<std::int64_t>& count_ref, CreateFnT&& create_fn, Args&&... args) noexcept;

        constexpr instance_tracker(instance_tracker&& other) noexcept;
        constexpr instance_tracker& operator=(instance_tracker&& other) noexcept;
        constexpr instance_tracker(const instance_tracker& other) noexcept;
        constexpr instance_tracker& operator=(const instance_tracker& other) noexcept;
        
        inline ~instance_tracker() noexcept;

    private:
        bool valid = true;
        std::atomic<std::int64_t>* count_ptr = nullptr;
        //inline static std::atomic<std::int64_t> count;
    };
}

#include "Duo2D/vulkan/core/instance_tracker.inl" 