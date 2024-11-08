#pragma once 
#include <concepts>

#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/vk_traits.hpp"

namespace d2d {
    template<impl::VulkanType VkTy>
    class pipeline_obj_base {
    public:
        constexpr pipeline_obj_base() noexcept = default;
        ~pipeline_obj_base() noexcept = default;

        constexpr VkTy* operator&() noexcept { return &handle; }
        constexpr VkTy const* operator&() const noexcept { return &handle; }
        constexpr operator VkTy() const noexcept { return handle; }
        constexpr explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }

    
    public:
        constexpr pipeline_obj_base(pipeline_obj_base&& other) noexcept : 
            handle(other.handle) { 
            other.handle = VK_NULL_HANDLE; 
        }
        constexpr pipeline_obj_base& operator=(pipeline_obj_base&& other) noexcept {
            handle = other.handle; 
            other.handle = VK_NULL_HANDLE;
            return *this;
        };

        constexpr pipeline_obj_base(const pipeline_obj_base& other) = default;
        constexpr pipeline_obj_base& operator=(const pipeline_obj_base& other) = default;
    
    protected:
        VkTy handle = VK_NULL_HANDLE;
    };
}

namespace d2d {
    template<impl::VulkanType VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class pipeline_obj : public pipeline_obj_base<VkTy> {
    public:
        constexpr pipeline_obj() noexcept = default;
        ~pipeline_obj() noexcept { if(this->handle) DeleterFn(this->handle, nullptr); };
    
    public:
        constexpr pipeline_obj(pipeline_obj&& other) noexcept = default;
        constexpr pipeline_obj& operator=(pipeline_obj&& other) noexcept = default;
        pipeline_obj(const pipeline_obj& other) = delete;
        pipeline_obj& operator=(const pipeline_obj& other) = delete;

    };

    template<impl::DependentVulkanType VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class pipeline_obj<VkTy, DeleterFn> : public pipeline_obj_base<VkTy> {
    public:
        constexpr pipeline_obj() noexcept = default;
        ~pipeline_obj() noexcept { if(this->handle) DeleterFn(dependent_handle, this->handle, nullptr); };
    
    public:
        constexpr pipeline_obj(pipeline_obj&& other) noexcept : 
            pipeline_obj_base<VkTy>(std::move(other)), dependent_handle(other.dependent_handle) {}
        constexpr pipeline_obj& operator=(pipeline_obj&& other) noexcept { 
            pipeline_obj_base<VkTy>::operator=(std::move(other));
            dependent_handle = other.dependent_handle;
            return *this;
        };

        pipeline_obj(const pipeline_obj& other) = delete;
        pipeline_obj& operator=(const pipeline_obj& other) = delete;


    protected:
        typename impl::vk_traits<VkTy>::dependent_type dependent_handle;
    };
}

namespace d2d {
    template<typename T, typename... Args>
    concept PipelineType = requires { {T::create(std::declval<Args>()...)} noexcept -> std::same_as<result<T>>; };
}