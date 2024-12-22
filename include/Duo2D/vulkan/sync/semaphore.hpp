#pragma once
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS_DEVICE(VkSemaphore);

namespace d2d {
    struct semaphore : vulkan_ptr<VkSemaphore, vkDestroySemaphore> {
        static result<semaphore> create(logical_device& device) noexcept;\
    };
}