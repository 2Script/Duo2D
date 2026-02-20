#pragma once 
#include <memory>
#include <type_traits>
#include <utility>

#include "Duo2D/core/error.hpp"
#include "Duo2D/traits/vk_traits.hpp"

namespace d2d::vk {
    template<impl::vulkan_like VkTy>
    class vulkan_ptr_base {
    public:
        using element_type = std::remove_pointer_t<VkTy>;
        using pointer = VkTy;
    public:
        constexpr vulkan_ptr_base() noexcept = default;
        constexpr ~vulkan_ptr_base() noexcept = default;

        constexpr VkTy      * operator&()      & noexcept { return &handle; }
        constexpr VkTy const* operator&() const& noexcept { return &handle; }
        constexpr operator VkTy      &()      & noexcept { return handle; }
        constexpr operator VkTy const&() const& noexcept { return handle; }
        constexpr explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }

    
    public:
        constexpr vulkan_ptr_base(vulkan_ptr_base&& other) noexcept : handle(std::exchange(other.handle, VK_NULL_HANDLE)) {}
        constexpr vulkan_ptr_base& operator=(vulkan_ptr_base&& other) noexcept {
            handle = std::exchange(other.handle, VK_NULL_HANDLE);
            return *this;
        };

        constexpr vulkan_ptr_base(const vulkan_ptr_base& other) = default;
        constexpr vulkan_ptr_base& operator=(const vulkan_ptr_base& other) = default;
    
    protected:
        VkTy handle = VK_NULL_HANDLE;
    };
}

namespace d2d::vk {
    template<impl::vulkan_like VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class vulkan_ptr : public vulkan_ptr_base<VkTy> {
    public:
        constexpr vulkan_ptr() noexcept = default;
        constexpr ~vulkan_ptr() noexcept { if(this->handle) DeleterFn(this->handle, nullptr); };
    
    public:
        constexpr vulkan_ptr(vulkan_ptr&& other) noexcept = default;
        constexpr vulkan_ptr& operator=(vulkan_ptr&& other) noexcept { 
            if(this->handle && this->handle != other.handle) 
                DeleterFn(this->handle, nullptr);
            vulkan_ptr_base<VkTy>::operator=(std::move(other));
            return *this;
        };
        vulkan_ptr(const vulkan_ptr& other) = delete;
        vulkan_ptr& operator=(const vulkan_ptr& other) = delete;

    };

    template<impl::dependent_vulkan_like VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class vulkan_ptr<VkTy, DeleterFn> : public vulkan_ptr_base<VkTy> {
    public:
        constexpr vulkan_ptr() noexcept = default;
        constexpr vulkan_ptr(std::shared_ptr<typename impl::vk_traits<VkTy>::dependent_type> dependent_ptr) noexcept :
			dependent_handle(dependent_ptr) {}
        constexpr ~vulkan_ptr() noexcept { if(this->handle) DeleterFn(*dependent_handle, this->handle, nullptr); };
    
    public:
        constexpr vulkan_ptr(vulkan_ptr&& other) noexcept = default;
        constexpr vulkan_ptr& operator=(vulkan_ptr&& other) noexcept { 
            if(this->handle && this->handle != other.handle) 
                DeleterFn(*dependent_handle, this->handle, nullptr);
            vulkan_ptr_base<VkTy>::operator=(std::move(other));
            dependent_handle = std::move(other.dependent_handle);
            return *this;
        };

        vulkan_ptr(const vulkan_ptr& other) = delete;
        vulkan_ptr& operator=(const vulkan_ptr& other) = delete;


    protected:
        std::shared_ptr<typename impl::vk_traits<VkTy>::dependent_type> dependent_handle;
    };

    template<impl::multiple_dependent_vulkan_like VkTy, typename impl::vk_traits<VkTy>::deleter_type& DeleterFn>
    class vulkan_ptr<VkTy, DeleterFn> : public vulkan_ptr_base<VkTy> {
    public:
        constexpr vulkan_ptr() noexcept = default;
        constexpr vulkan_ptr(std::shared_ptr<typename impl::vk_traits<VkTy>::dependent_type> dependent_ptr, std::shared_ptr<typename impl::vk_traits<VkTy>::auxiliary_type> aux_ptr) noexcept :
			dependent_handle(dependent_ptr), aux_handle(aux_ptr) {}
        constexpr ~vulkan_ptr() noexcept { if(this->handle) DeleterFn(*dependent_handle, *aux_handle, 1, &this->handle); };
    
    public:
        constexpr vulkan_ptr(vulkan_ptr&& other) noexcept = default;
        constexpr vulkan_ptr& operator=(vulkan_ptr&& other) noexcept { 
            if(this->handle && this->handle != other.handle) 
                DeleterFn(*dependent_handle, *aux_handle, 1, &this->handle);
            vulkan_ptr_base<VkTy>::operator=(std::move(other));
            dependent_handle = std::move(other.dependent_handle);
            aux_handle = std::move(other.aux_handle);
            return *this;
        };

        vulkan_ptr(const vulkan_ptr& other) = delete;
        vulkan_ptr& operator=(const vulkan_ptr& other) = delete;


    protected:
        std::shared_ptr<typename impl::vk_traits<VkTy>::dependent_type> dependent_handle;
        std::shared_ptr<typename impl::vk_traits<VkTy>::auxiliary_type> aux_handle;
    };
}