#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/core/command_family.hpp"
#include <vulkan/vulkan_core.h>

namespace acma::vk {
    result<command_pool> command_pool::create(command_family_t family, std::shared_ptr<logical_device> logi_device, physical_device* phys_device) noexcept {
        command_pool ret{};
        ret.dependent_handle = logi_device;

        VkCommandPoolCreateInfo command_pool_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = phys_device->queue_family_infos[family].index,
        };

        __D2D_VULKAN_VERIFY(vkCreateCommandPool(*logi_device, &command_pool_info, nullptr, &ret.handle));
        return ret;
    }
}