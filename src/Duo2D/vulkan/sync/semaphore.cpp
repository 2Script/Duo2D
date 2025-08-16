#include "Duo2D/vulkan/sync/semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<semaphore> semaphore::create(std::shared_ptr<logical_device> device, VkSemaphoreType semaphore_type) noexcept {
        semaphore ret{};
        ret.dependent_handle = device;

        VkSemaphoreTypeCreateInfo semaphore_type_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = NULL,
            .semaphoreType = semaphore_type,
            .initialValue = 0,
        };

        VkSemaphoreCreateInfo semaphore_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphore_type_info,
        };

        __D2D_VULKAN_VERIFY(vkCreateSemaphore(*device, &semaphore_info, nullptr, &ret.handle));
        return ret;
    }
}
