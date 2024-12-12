#pragma once 
#include <concepts>

#include "Duo2D/error.hpp"
#include "Duo2D/traits/vk_traits.hpp"

namespace d2d {
    template<impl::VulkanType VkTy>
    class vulkan_ptr_base {
    public:
        constexpr vulkan_ptr_base() noexcept = default;
        ~vulkan_ptr_base() noexcept = default;

        constexpr VkTy* operator&() noexcept { return &handle; }
        constexpr VkTy const* operator&() const noexcept { return &handle; }
        constexpr operator VkTy() const noexcept { return handle; }
        constexpr explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }

    
    public:
        constexpr vulkan_ptr_base(vulkan_ptr_base&& other) noexcept : 
            handle(other.handle) { 
            other.handle = VK_NULL_HANDLE; 
        }
        constexpr vulkan_ptr_base& operator=(vulkan_ptr_base&& other) noexcept {
            handle = other.handle; 
            other.handle = VK_NULL_HANDLE;
            return *this;
        };

        constexpr vulkan_ptr_base(const vulkan_ptr_base& other) = default;
        constexpr vulkan_ptr_base& operator=(const vulkan_ptr_base& other) = default;
    
    protected:
        VkTy handle = VK_NULL_HANDLE;
    };
}

namespace d2d {
    template<impl::VulkanType VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class vulkan_ptr : public vulkan_ptr_base<VkTy> {
    public:
        constexpr vulkan_ptr() noexcept = default;
        ~vulkan_ptr() noexcept { if(this->handle) DeleterFn(this->handle, nullptr); };
    
    public:
        constexpr vulkan_ptr(vulkan_ptr&& other) noexcept = default;
        constexpr vulkan_ptr& operator=(vulkan_ptr&& other) noexcept = default;
        vulkan_ptr(const vulkan_ptr& other) = delete;
        vulkan_ptr& operator=(const vulkan_ptr& other) = delete;

    };

    template<impl::DependentVulkanType VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class vulkan_ptr<VkTy, DeleterFn> : public vulkan_ptr_base<VkTy> {
    public:
        constexpr vulkan_ptr() noexcept = default;
        ~vulkan_ptr() noexcept { if(this->handle) DeleterFn(dependent_handle, this->handle, nullptr); };
    
    public:
        constexpr vulkan_ptr(vulkan_ptr&& other) noexcept : 
            vulkan_ptr_base<VkTy>(std::move(other)), dependent_handle(other.dependent_handle) {}
        constexpr vulkan_ptr& operator=(vulkan_ptr&& other) noexcept { 
            vulkan_ptr_base<VkTy>::operator=(std::move(other));
            dependent_handle = other.dependent_handle;
            return *this;
        };

        vulkan_ptr(const vulkan_ptr& other) = delete;
        vulkan_ptr& operator=(const vulkan_ptr& other) = delete;


    protected:
        typename impl::vk_traits<VkTy>::dependent_type dependent_handle;
    };
}