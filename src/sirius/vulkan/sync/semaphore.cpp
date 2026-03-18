#include "sirius/vulkan/sync/semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace acma::vk {
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

namespace acma::vk {
	result<void> semaphore::wait(sl::uint64_t value, sl::uint64_t timeout) const noexcept {
		VkSemaphoreWaitInfo semaphore_post_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &handle,
			.pValues = &value
		};
		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*dependent_handle, &semaphore_post_wait_info, timeout));
		return {};
	}
}