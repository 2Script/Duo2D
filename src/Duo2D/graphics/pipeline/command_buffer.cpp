#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/window.hpp"
#include "Duo2D/graphics/prim/viewport.hpp"
#include "Duo2D/arith/size.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<command_buffer> command_buffer::create(logical_device& device, command_pool& pool) noexcept {
        command_buffer ret{};

        VkCommandBufferAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        __D2D_VULKAN_VERIFY(vkAllocateCommandBuffers(device, &alloc_info, &ret.handle));
        return ret;
    }
}

namespace d2d {
    result<void> command_buffer::record(const window& w, std::vector<VkBuffer>& vertex_buffers, shader_buffer& index_buffer, std::vector<std::size_t>& offsets, std::size_t index_count, std::uint32_t image_index) const noexcept {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));

        //Set render pass
        constexpr static VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = w._render_pass,
            .framebuffer = w._swap_chain.framebuffers[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = static_cast<VkExtent2D>(w._swap_chain.extent),
            },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };
        vkCmdBeginRenderPass(handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        //Set pipeline
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, w._pipeline);

        //Set viewport
        viewport v({}, static_cast<size2<float>>(w._swap_chain.extent));
        vkCmdSetViewport(handle, 0, 1, &v);

        //Set viewport crop
        VkRect2D scissor{{}, static_cast<VkExtent2D>(w._swap_chain.extent)};
        vkCmdSetScissor(handle, 0, 1, &scissor);

        //Bind buffers
        vkCmdBindVertexBuffers(handle, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
        //vkCmdBindVertexBuffers(handle, 0, 2, vertex_buffers.data(), offsets_arr.data());
        vkCmdBindIndexBuffer(handle, static_cast<VkBuffer>(index_buffer), 0, VK_INDEX_TYPE_UINT32);

        //Draw verticies
        vkCmdDrawIndexed(handle, index_count, 1, 0, 0, 0);

        
        vkCmdEndRenderPass(handle);
        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        return result<void>{std::in_place_type<void>};
    }


    result<void> command_buffer::reset() const noexcept {
        __D2D_VULKAN_VERIFY(vkResetCommandBuffer(handle, 0));
        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    result<void> command_buffer::copy(buffer& dest, const buffer& src, std::size_t size) const noexcept {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer(handle, src, dest, 1, &copy_region);

        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        return result<void>{std::in_place_type<void>};
    }
}