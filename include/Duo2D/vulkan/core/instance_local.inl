#pragma once
#include "Duo2D/vulkan/core/instance_local.hpp"


namespace d2d::impl {
    template<typename InstanceT, typename T, auto DestroyFn> 
    template<typename CreateFnT, typename... Args> requires instance_local<InstanceT, T, DestroyFn>::has_data
    constexpr instance_local<InstanceT, T, DestroyFn>::instance_local(CreateFnT&& create_fn, Args&&... args) noexcept {
        if(instance_count == 0)
            this->data = std::forward<CreateFnT>(create_fn)(std::forward<Args>(args)...);
        ++instance_count;
    }


    template<typename InstanceT, typename T, auto DestroyFn> 
    constexpr instance_local<InstanceT, T, DestroyFn>::instance_local(instance_local&& other) noexcept : 
        impl::dependent_member<T>(std::move(other)) {
        ++instance_count;
    }

    template<typename InstanceT, typename T, auto DestroyFn> 
    constexpr instance_local<InstanceT, T, DestroyFn>& instance_local<InstanceT, T, DestroyFn>::operator=(instance_local&& other) noexcept {
        impl::dependent_member<T>::operator=(std::move(other));
        ++instance_count;
        return *this;
    }


    template<typename InstanceT, typename T, auto DestroyFn> 
    constexpr instance_local<InstanceT, T, DestroyFn>::instance_local(const instance_local& other) noexcept : 
        impl::dependent_member<T>(other) {
        ++instance_count;
    }

    template<typename InstanceT, typename T, auto DestroyFn> 
    constexpr instance_local<InstanceT, T, DestroyFn>& instance_local<InstanceT, T, DestroyFn>::operator=(const instance_local& other) noexcept {
        impl::dependent_member<T>::operator=(other);
        ++instance_count;
        return *this;
    }


    template<typename InstanceT, typename T, auto DestroyFn> 
    instance_local<InstanceT, T, DestroyFn>::~instance_local() noexcept {
        //not thread safe
        if(instance_count > 0) --instance_count;
        if(instance_count > 0) return;
        if constexpr(has_data) DestroyFn(this->data);
        else DestroyFn();
    }
}