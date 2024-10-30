#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkRenderPass);

namespace d2d {
    struct render_pass : pipeline_obj<VkRenderPass, vkDestroyRenderPass> {
        static result<render_pass> create(logical_device& device) noexcept;
    };
}
