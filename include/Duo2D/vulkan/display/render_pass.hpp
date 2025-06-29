#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkRenderPass);

namespace d2d::vk {
    struct render_pass : vulkan_ptr<VkRenderPass, vkDestroyRenderPass> {
        static result<render_pass> create(std::shared_ptr<logical_device> device) noexcept;
    };
}
