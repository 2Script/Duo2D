#pragma once
#include <cstddef>
#include <type_traits>
#include "Duo2D/error.hpp"
#include "Duo2D/traits/dependent_member.hpp"


namespace d2d::impl {
    template<typename InstanceT, typename T, auto DestroyFn> 
    class instance_local : public dependent_member<T> {
        constexpr static bool has_data = !std::is_same_v<T, void>;
    public:
        constexpr instance_local() noexcept = default;
        template<typename CreateFnT, typename... Args> requires has_data
        constexpr instance_local(CreateFnT&& create_fn, Args&&... args) noexcept;
    
        constexpr instance_local(instance_local&& other) noexcept;
        constexpr instance_local& operator=(instance_local&& other) noexcept;
        constexpr instance_local(const instance_local& other) noexcept;
        constexpr instance_local& operator=(const instance_local& other) noexcept;

        ~instance_local() noexcept;

    public:
        inline static std::size_t instance_count = 0;
    };
}

#include "Duo2D/vulkan/core/instance_local.inl"