#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/size.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<command_buffer> command_buffer::create(logical_device& device, const command_pool& pool) noexcept {
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
    result<void> command_buffer::begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));

        //Set render pass
        constexpr static VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = window_render_pass,
            .framebuffer = window_swap_chain.framebuffers[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = static_cast<VkExtent2D>(window_swap_chain.extent),
            },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };
        vkCmdBeginRenderPass(handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        //Set viewport
        const rect<float> screen_bounds = {{}, static_cast<size2<float>>(window_swap_chain.extent)};
        VkViewport v{screen_bounds.x(), screen_bounds.y(), screen_bounds.width(), screen_bounds.height(), 0.f, 1.f};
        vkCmdSetViewport(handle, 0, 1, &v);

        //Set viewport crop
        VkRect2D scissor{{}, static_cast<VkExtent2D>(window_swap_chain.extent)};
        vkCmdSetScissor(handle, 0, 1, &scissor);

        return result<void>{std::in_place_type<void>};
    }

    result<void> command_buffer::end() const noexcept {
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