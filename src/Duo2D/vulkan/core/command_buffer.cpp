#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<command_buffer> command_buffer::create(logical_device& device, const command_pool& pool) noexcept {
        command_buffer ret{};
        //ret.dependent_handle = device;
        //ret.aux_handle = pool;

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
    result<void> command_buffer::render_begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept {
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

        return {};
    }

    result<void> command_buffer::render_end() const noexcept {
        vkCmdEndRenderPass(handle);
        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        return {};
    }

    result<void> command_buffer::reset() const noexcept {
        __D2D_VULKAN_VERIFY(vkResetCommandBuffer(handle, 0));
        return {};
    }
}

namespace d2d {
    result<void> command_buffer::generic_begin() const noexcept {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));
        return {};
    }


    void command_buffer::copy_alike(buffer& dest, const buffer& src, std::size_t size, std::size_t offset) const noexcept {
        VkBufferCopy copy_region{ 
            .srcOffset = offset,
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(handle, src, dest, 1, &copy_region);
    }

    void command_buffer::copy_alike(image& dest, image& src, extent2 size) const noexcept {
        VkImageLayout original_dest_layout = dest.layout();
        VkImageLayout original_src_layout = src.layout();

        VkImageCopy copy_region{
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .srcOffset = {0, 0, 0},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .dstOffset = {0, 0, 0},
            .extent = {size.width(), size.height(), 1}
        };
        transition_image(src,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, original_src_layout );
        transition_image(dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, original_dest_layout);
        vkCmdCopyImage(handle, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        //transition_image(src,  original_src_layout,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        transition_image(dest, original_src_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }

    void command_buffer::copy_buffer_to_image(image& dest, const buffer& src, extent2 image_size, std::size_t buffer_offset, std::uint32_t array_idx) const noexcept {
        VkBufferImageCopy copy_region{
            .bufferOffset = buffer_offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,

            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, array_idx, 1},
            .imageOffset = {0, 0, 0},
            .imageExtent = {image_size.width(), image_size.height(), 1},
        };
        
        vkCmdCopyBufferToImage(handle, src, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    }

    void command_buffer::copy_buffer_to_image(image& dest, const buffer& src, std::span<const VkBufferImageCopy> copy_regions) const noexcept {
        vkCmdCopyBufferToImage(handle, src, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_regions.size(), copy_regions.data());
    }


    void command_buffer::transition_image(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count) const noexcept {
        constexpr auto access_mask_from_layout = [](VkImageLayout layout) noexcept -> VkAccessFlags {
            switch(layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_ACCESS_TRANSFER_READ_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
            default:                                       return 0;
            };
        };
        constexpr auto pipeline_stage_from_layout = [](VkImageLayout layout) noexcept -> VkPipelineStageFlags {
            switch(layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:     return VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            default:                                       return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            };
        };
        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = access_mask_from_layout(old_layout),
            .dstAccessMask = access_mask_from_layout(new_layout),
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = img,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = image_count,
            },
        };

        vkCmdPipelineBarrier(handle, pipeline_stage_from_layout(old_layout), pipeline_stage_from_layout(new_layout), 0, 0, nullptr, 0, nullptr, 1, &barrier);
        img.layout() = new_layout;
    }


    result<void> command_buffer::generic_end(logical_device& device, const command_pool& pool) const noexcept {
        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &handle;
        vkQueueSubmit(device.queues[queue_family::graphics], 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.queues[queue_family::graphics]);

        vkFreeCommandBuffers(device, pool, 1, &handle);
        return {};
    }
}