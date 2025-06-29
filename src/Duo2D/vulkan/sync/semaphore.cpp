#include "Duo2D/vulkan/sync/semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<semaphore> semaphore::create(std::shared_ptr<logical_device> device) noexcept {
        semaphore ret{};
        ret.dependent_handle = device;

        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        __D2D_VULKAN_VERIFY(vkCreateSemaphore(*device, &semaphore_info, nullptr, &ret.handle));
        return ret;
    }
}
