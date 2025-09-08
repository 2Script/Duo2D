#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/arith/size.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkFramebuffer);

namespace d2d::vk {
    struct framebuffer : vulkan_ptr<VkFramebuffer, vkDestroyFramebuffer> {
        static result<framebuffer> create(std::shared_ptr<logical_device> device, image_view& color_attachment, image_view const& depth_attachment, render_pass& associated_render_pass, extent2 size) noexcept;
    };
}
