#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<render_pass> render_pass::create(logical_device& logi_device) noexcept {
        render_pass ret{};
        ret.dependent_handle = logi_device;

        //Specify load/store of color and stencil data (TEMP: specify that images are to be presented in swap chain)
        VkAttachmentDescription attachment{};
        {
        attachment.format = logi_device.format.format_id;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //TEMP
        }

        //Create supasses (TEMP: create one subpass to be used as a color buffer)
        VkSubpassDescription subpass{};
        VkAttachmentReference attachment_ref{};
        {
        attachment_ref.attachment = 0; //output location 0 in shader
        attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachment_ref;
        }


        //Create the render pass itself
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        __D2D_VULKAN_VERIFY(vkCreateRenderPass(logi_device, &render_pass_info, nullptr, &ret.handle));

        return ret;
    }
}
