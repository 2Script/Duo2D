#pragma once
#include "Duo2D/vulkan/core/instance_tracker.hpp"


namespace d2d::impl {
    template<typename T, auto DestroyFn>
    template<typename CreateFnT, typename... Args>
    constexpr result<void> instance_tracker<T, DestroyFn>::initialize(std::atomic<std::int64_t>& count_ref, CreateFnT&& create_fn, Args&&... args) noexcept {
        count_ptr = &count_ref;
        if((*count_ptr)++ != 0) return {};

        if constexpr(has_data)
            RESULT_TRY_MOVE(this->data, std::forward<CreateFnT>(create_fn)(std::forward<Args>(args)...))
        else
            RESULT_VERIFY(std::forward<CreateFnT>(create_fn)(std::forward<Args>(args)...))
        return {};
    }


    template<typename T, auto DestroyFn> 
    constexpr instance_tracker<T, DestroyFn>::instance_tracker(instance_tracker&& other) noexcept : 
        impl::dependent_static_member<T, DestroyFn>(std::move(other)), valid(true), count_ptr(other.count_ptr) {
        other.valid = false;
    }

    template<typename T, auto DestroyFn> 
    constexpr instance_tracker<T, DestroyFn>& instance_tracker<T, DestroyFn>::operator=(instance_tracker&& other) noexcept {
        impl::dependent_static_member<T, DestroyFn>::operator=(std::move(other));
        valid = true;
        other.valid = false;
        count_ptr = other.count_ptr;
        return *this;
    }


    template<typename T, auto DestroyFn> 
    constexpr instance_tracker<T, DestroyFn>::instance_tracker(const instance_tracker& other) noexcept : 
        impl::dependent_static_member<T, DestroyFn>(other), valid(true), count_ptr(other.count_ptr) {
        ++(*count_ptr);
    }

    template<typename T, auto DestroyFn> 
    constexpr instance_tracker<T, DestroyFn>& instance_tracker<T, DestroyFn>::operator=(const instance_tracker& other) noexcept {
        impl::dependent_static_member<T, DestroyFn>::operator=(other);
        count_ptr = other.count_ptr;
        ++(*count_ptr);
        return *this;
    }


    template<typename T, auto DestroyFn> 
    instance_tracker<T, DestroyFn>::~instance_tracker() noexcept {
        //const std::lock_guard<std::mutex> cnt_lck(instance_count<InstanceT>_mutex());
        if(!valid || !count_ptr || --(*count_ptr) != 0) return;
        if constexpr(has_data) DestroyFn(this->data);
        else DestroyFn();
        //DestroyFn();
    }
}