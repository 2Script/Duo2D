#include "Duo2D/graphics/pipeline/framebuffer.hpp"
#include "Duo2D/error.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<framebuffer> framebuffer::create(logical_device& device, image_view& attachment, render_pass& associated_render_pass, extent2 size) noexcept {
        framebuffer ret{};
        ret.dependent_handle = device;

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = associated_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &attachment;
        framebuffer_info.width = size.width();
        framebuffer_info.height = size.height();
        framebuffer_info.layers = 1;

        __D2D_VULKAN_VERIFY(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &ret.handle));
        return ret;
    }
}