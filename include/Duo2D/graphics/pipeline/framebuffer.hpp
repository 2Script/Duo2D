#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include "Duo2D/graphics/pipeline/image_view.hpp"
#include "Duo2D/arith/size.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkFramebuffer);

namespace d2d {
    struct framebuffer : pipeline_obj<VkFramebuffer, vkDestroyFramebuffer> {
        static result<framebuffer> create(logical_device& device, image_view& attachment, render_pass& associated_render_pass, extent2 size) noexcept;
    };
}
