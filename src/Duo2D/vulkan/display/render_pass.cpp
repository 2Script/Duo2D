#include "Duo2D/vulkan/display/render_pass.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<render_pass> render_pass::create(std::shared_ptr<logical_device> logi_device) noexcept {
        render_pass ret{};
        ret.dependent_handle = logi_device;

        //Specify load/store of color and stencil data (TEMP: specify that images are to be presented in swap chain)
        VkAttachmentDescription attachment{
            .format = logi_device->format.format_id,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, //TEMP
        };

        //Create subpasses (TEMP: create one subpass to be used as a color buffer)
        VkAttachmentReference attachment_ref{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachment_ref,
        };

        //Create subpass dependencies
        std::array<VkSubpassDependency, 1> dependencies{{
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
        }};

        //Create the render pass itself
        VkRenderPassCreateInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = dependencies.size(),
            .pDependencies = dependencies.data(),
        };

        __D2D_VULKAN_VERIFY(vkCreateRenderPass(*logi_device, &render_pass_info, nullptr, &ret.handle));

        return ret;
    }
}
