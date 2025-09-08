#include "Duo2D/vulkan/display/framebuffer.hpp"
#include "Duo2D/core/error.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<framebuffer> framebuffer::create(std::shared_ptr<logical_device> device, image_view& color_attachment, image_view const& depth_attachment, render_pass& associated_render_pass, extent2 size) noexcept {
        framebuffer ret{};
        ret.dependent_handle = device;

        std::array<VkImageView, 2> attachments{color_attachment, depth_attachment};
        VkFramebufferCreateInfo framebuffer_info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = associated_render_pass,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = size.width(),
            .height = size.height(),
            .layers = 1,
        };

        __D2D_VULKAN_VERIFY(vkCreateFramebuffer(*device, &framebuffer_info, nullptr, &ret.handle));
        return ret;
    }
}