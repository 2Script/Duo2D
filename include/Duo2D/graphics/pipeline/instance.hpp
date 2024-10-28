#pragma once
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/vk_traits.hpp"
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS(VkInstance);

namespace d2d {
    struct instance : pipeline_obj<VkInstance, vkDestroyInstance> {
        static result<instance> create(VkApplicationInfo& app_info) noexcept;
    };
}
