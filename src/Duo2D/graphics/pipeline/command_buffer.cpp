#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/window.hpp"
#include "Duo2D/graphics/viewport.hpp"
#include "Duo2D/prim/rect.hpp"
#include "Duo2D/prim/size.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<command_buffer> command_buffer::create(logical_device& device, command_pool& pool) noexcept {
        command_buffer ret{};

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        __D2D_VULKAN_VERIFY(vkAllocateCommandBuffers(device, &alloc_info, &ret.handle));
        return ret;
    }
}

namespace d2d {
    result<void> command_buffer::record(const window& w, std::uint32_t image_index) const noexcept {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));

        //Set render pass
        constexpr static VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = w._render_pass;
        render_pass_info.framebuffer = w._swap_chain.framebuffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = static_cast<VkExtent2D>(w._swap_chain.extent);
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;
        vkCmdBeginRenderPass(handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        //Set pipeline
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, w._pipeline);

        //Set viewport
        viewport v({}, static_cast<size2<float>>(w._swap_chain.extent));
        vkCmdSetViewport(handle, 0, 1, &v);

        //Set viewport crop
        VkRect2D scissor{{}, static_cast<VkExtent2D>(w._swap_chain.extent)};
        vkCmdSetScissor(handle, 0, 1, &scissor);

        //Draw 3 verticies
        vkCmdDraw(handle, 3, 1, 0, 0);

        
        vkCmdEndRenderPass(handle);
        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        return result<void>{std::in_place_type<void>};
    }


    result<void> command_buffer::reset() const noexcept {
        __D2D_VULKAN_VERIFY(vkResetCommandBuffer(handle, 0));
        return result<void>{std::in_place_type<void>};
    }
}