#pragma once
#include <cstddef>
#include <type_traits>
#include "Duo2D/core/error.hpp"
#include "Duo2D/traits/dependent_member.hpp"


namespace d2d::impl {
    template<typename InstanceT, typename T, auto DestroyFn> 
    class instance_tracker : public dependent_member<T> {
        constexpr static bool has_data = !std::is_same_v<T, void>;
    public:
        constexpr instance_tracker() noexcept = default;
        template<typename CreateFnT, typename... Args> requires has_data
        constexpr instance_tracker(CreateFnT&& create_fn, Args&&... args) noexcept;
    
        constexpr instance_tracker(instance_tracker&& other) noexcept;
        constexpr instance_tracker& operator=(instance_tracker&& other) noexcept;
        constexpr instance_tracker(const instance_tracker& other) noexcept;
        constexpr instance_tracker& operator=(const instance_tracker& other) noexcept;

        ~instance_tracker() noexcept;

    public:
        inline static std::size_t instance_count = 0;
    };
}

#include "Duo2D/vulkan/core/instance_tracker.inl"