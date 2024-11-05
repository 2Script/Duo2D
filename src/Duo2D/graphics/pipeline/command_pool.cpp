#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/hardware/device/queue_family.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<command_pool> command_pool::create(logical_device& logi_device, physical_device& phys_device) noexcept {
        command_pool ret{};
        ret.dependent_handle = logi_device;

        VkCommandPoolCreateInfo command_pool_info{};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex = *phys_device.queue_family_idxs[queue_family::graphics];

        __D2D_VULKAN_VERIFY(vkCreateCommandPool(logi_device, &command_pool_info, nullptr, &ret.handle));
        return ret;
    }
}