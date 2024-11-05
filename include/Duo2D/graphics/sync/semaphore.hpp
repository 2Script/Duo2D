#pragma once
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS_DEVICE(VkSemaphore);

namespace d2d {
    struct semaphore : pipeline_obj<VkSemaphore, vkDestroySemaphore> {
        static result<semaphore> create(logical_device& device) noexcept;\
    };
}
