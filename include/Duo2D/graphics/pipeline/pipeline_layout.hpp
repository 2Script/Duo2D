#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipelineLayout);

namespace d2d {
    struct pipeline_layout : pipeline_obj<VkPipelineLayout, vkDestroyPipelineLayout> {
        static result<pipeline_layout> create(logical_device& device, descriptor_set_layout& set_layout) noexcept;
        static result<pipeline_layout> create(logical_device& device) noexcept;
    };
}
