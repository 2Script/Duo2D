#include "Duo2D/vulkan/sync/fence.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<fence> fence::create(std::shared_ptr<logical_device> device) noexcept {
        fence ret{};
        ret.dependent_handle = device;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        __D2D_VULKAN_VERIFY(vkCreateFence(*device, &fence_info, nullptr, &ret.handle));
        return ret;
    }
}


namespace d2d::vk {
    result<void> fence::wait(std::uint64_t timeout) const noexcept {
        __D2D_VULKAN_VERIFY(vkWaitForFences(*dependent_handle, 1, &handle, VK_TRUE, timeout));
        return {};
    }

    result<void> fence::reset() const noexcept {
        __D2D_VULKAN_VERIFY(vkResetFences(*dependent_handle, 1, &handle));
        return {};
    }
}
