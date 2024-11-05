#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkCommandPool);


namespace d2d {
    struct command_pool : pipeline_obj<VkCommandPool, vkDestroyCommandPool> {
        static result<command_pool> create(logical_device& logi_deivce, physical_device& phys_device) noexcept;
    };
}
