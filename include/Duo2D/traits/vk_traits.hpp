#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>


namespace d2d::vk::impl { 
    template<typename VkTy> struct vk_traits{
        static_assert(!std::is_same_v<VkTy, VkTy>, "vk_traits<VkType> specialization has not been defined for the given VkType!");
    }; 

    template<typename VkTy>
    concept vulkan_like = std::is_pointer_v<VkTy> && std::is_same_v<std::remove_cvref_t<VkTy>, VkTy>;

    template<typename VkTy>
    concept dependent_vulkan_like = vulkan_like<VkTy> && requires { typename vk_traits<VkTy>::dependent_type; };

    template<typename VkTy>
    concept multiple_dependent_vulkan_like = dependent_vulkan_like<VkTy> && requires { typename vk_traits<VkTy>::auxiliary_type; };
}


#define __D2D_DECLARE_VK_TRAITS_DEVICE(type) \
namespace d2d::vk::impl { \
    template<> struct vk_traits<type>{ \
        using dependent_type = vk::logical_device; \
        using deleter_type = void(typename dependent_type::pointer, type, const VkAllocationCallbacks*); \
    }; \
}

#define __D2D_DECLARE_VK_TRAITS_DEVICE_AUX(type, aux_dependent_type) \
namespace d2d::vk::impl { \
    template<> struct vk_traits<type>{ \
        using dependent_type = vk::logical_device; \
        using auxiliary_type = aux_dependent_type; \
        using deleter_type = void(typename dependent_type::pointer, typename auxiliary_type::pointer, std::uint32_t, type const*); \
    }; \
}

#define __D2D_DECLARE_VK_TRAITS_INST(type) \
namespace d2d::vk::impl { \
    template<> struct vk_traits<type>{ \
        using dependent_type = vk::instance; \
        using deleter_type = void(typename dependent_type::pointer, type, const VkAllocationCallbacks*); \
    }; \
}

#define __D2D_DECLARE_VK_TRAITS(type) \
namespace d2d::vk::impl { \
    template<> struct vk_traits<type>{ \
        using deleter_type = void(type, const VkAllocationCallbacks*); \
    }; \
}
