#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/framebuffer.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<command_buffer> command_buffer::create(std::shared_ptr<logical_device> device, std::shared_ptr<command_pool> pool) noexcept {
        command_buffer ret{};
        ret.logi_device_ptr = device;
        ret.cmd_pool_ptr = pool;

        VkCommandBufferAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = *pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        __D2D_VULKAN_VERIFY(vkAllocateCommandBuffers(*device, &alloc_info, &ret.handle));
        return ret;
    }
}


namespace d2d::vk {
    result<void> command_buffer::begin(bool one_time) const noexcept {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = one_time ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0U,
        };
        __D2D_VULKAN_VERIFY(vkBeginCommandBuffer(handle, &begin_info));
        return {};
    }

    result<void> command_buffer::end() const noexcept {
        __D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
        return {};
    }


    result<void> command_buffer::reset() const noexcept {
        __D2D_VULKAN_VERIFY(vkResetCommandBuffer(handle, 0));
        return {};
    }

    result<void> command_buffer::submit(queue_family::id family, std::span<const semaphore_submit_info> wait_semaphore_infos, std::span<const semaphore_submit_info> signal_semaphore_infos, VkFence out_fence) const noexcept {
        VkCommandBufferSubmitInfo cmd_buff_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = handle,
            .deviceMask = 0,
        };
        VkSubmitInfo2 submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .waitSemaphoreInfoCount = static_cast<std::uint32_t>(wait_semaphore_infos.size()),
            .pWaitSemaphoreInfos = wait_semaphore_infos.data(),
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cmd_buff_info,
            .signalSemaphoreInfoCount = static_cast<std::uint32_t>(signal_semaphore_infos.size()),
            .pSignalSemaphoreInfos = signal_semaphore_infos.data(),
        };
        __D2D_VULKAN_VERIFY(vkQueueSubmit2(logi_device_ptr->queues[family], 1, &submit_info, out_fence));
        return {};
    }

    result<void> command_buffer::wait(queue_family::id family) const noexcept {
        __D2D_VULKAN_VERIFY(vkQueueWaitIdle(logi_device_ptr->queues[family]));
        return {};
    }

    void command_buffer::free() const noexcept {
        return vkFreeCommandBuffers(*logi_device_ptr, *cmd_pool_ptr, 1, &handle);
    }
}

namespace d2d::vk {
    void command_buffer::render_begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::span<framebuffer> framebuffers, std::uint32_t image_index) const noexcept {

        //Set render pass
        constexpr static std::array<VkClearValue, 2> clear_colors = {{{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}}, {.depthStencil = {0.0f, 0}}}};
        VkRenderPassBeginInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = window_render_pass,
            .framebuffer = framebuffers[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = static_cast<VkExtent2D>(window_swap_chain.extent()),
            },
            .clearValueCount = clear_colors.size(),
            .pClearValues = clear_colors.data(),
        };
        vkCmdBeginRenderPass(handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        //Set viewport
        const rect<float> screen_bounds = {{}, static_cast<size2<float>>(window_swap_chain.extent())};
        VkViewport v{screen_bounds.x(), screen_bounds.y(), screen_bounds.width(), screen_bounds.height(), 0.f, 1.f};
        vkCmdSetViewport(handle, 0, 1, &v);

        //Set viewport crop
        VkRect2D scissor{{}, static_cast<VkExtent2D>(window_swap_chain.extent())};
        vkCmdSetScissor(handle, 0, 1, &scissor);
    }

    void command_buffer::render_end() const noexcept {
        vkCmdEndRenderPass(handle);
        //__D2D_VULKAN_VERIFY(vkEndCommandBuffer(handle));
    }
}


namespace d2d::vk {
    void command_buffer::pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept {
        VkDependencyInfo barrier_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0,
            .memoryBarrierCount = static_cast<std::uint32_t>(global_barriers.size()),
            .pMemoryBarriers = global_barriers.data(),
            .bufferMemoryBarrierCount = static_cast<std::uint32_t>(buffer_barriers.size()),
            .pBufferMemoryBarriers = buffer_barriers.data(),
            .imageMemoryBarrierCount = static_cast<std::uint32_t>(image_barriers.size()),
            .pImageMemoryBarriers = image_barriers.data(),
        };
        vkCmdPipelineBarrier2(handle, &barrier_info);
    }

    void command_buffer::image_transition(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count) const noexcept {
        constexpr auto image_barrier_mask = [](VkImageLayout layout) noexcept -> std::pair<VkPipelineStageFlagBits2, VkAccessFlagBits2> {
            auto it = image_barrier_map.find(layout);
            if (it == image_barrier_map.end()) return {VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE};
            return it->second;
        };
        VkImageMemoryBarrier2 image_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = image_barrier_mask(old_layout).first,
            .srcAccessMask = image_barrier_mask(old_layout).second,
            .dstStageMask = image_barrier_mask(new_layout).first,
            .dstAccessMask = image_barrier_mask(new_layout).second,
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

        pipeline_barrier({}, {}, std::span<const VkImageMemoryBarrier2, 1>{&image_barrier, 1});
        img.layout() = new_layout;
    }
}


namespace d2d::vk {
    void command_buffer::copy_alike(buffer& dst, const buffer& src, std::size_t size, std::size_t offset) const noexcept {
        std::array<VkBufferMemoryBarrier2, 2> before_barriers {{
        //src barrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = src,
            .offset = offset,
            .size = size
        },
        //dst barrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = dst,
            .offset = offset,
            .size = size
        }
        }};
        pipeline_barrier({}, before_barriers, {});
        

        VkBufferCopy copy_region{ 
            .srcOffset = offset,
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(handle, src, dst, 1, &copy_region);


        std::array<VkBufferMemoryBarrier2, 2> after_barriers {{
        //src barrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            .dstAccessMask = 0,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = src,
            .offset = offset,
            .size = size
        },
        //dst barrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            .dstAccessMask = 
                (dst.usage_flags() & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT  ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : 0U) | 
                (dst.usage_flags() & VK_BUFFER_USAGE_INDEX_BUFFER_BIT   ? VK_ACCESS_INDEX_READ_BIT            : 0U) | 
                (dst.usage_flags() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ? VK_ACCESS_UNIFORM_READ_BIT          : 0U),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = dst,
            .offset = offset,
            .size = size
        }
        }};
        pipeline_barrier({}, after_barriers, {});
    }

    void command_buffer::copy_alike(image& dst, image& src, extent2 size) const noexcept {
        VkImageLayout original_dst_layout = dst.layout();
        VkImageLayout original_src_layout = src.layout();
        const std::uint32_t layer_count = std::min(dst.count(), src.count());

        VkImageCopy copy_region{
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layer_count},
            .srcOffset = {0, 0, 0},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layer_count},
            .dstOffset = {0, 0, 0},
            .extent = {size.width(), size.height(), 1}
        };
        image_transition(src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, original_src_layout, src.count());
        image_transition(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, original_dst_layout, dst.count());
        vkCmdCopyImage(handle, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        //image_transition(src,  original_src_layout,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src.count());
        image_transition(dst, original_src_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst.count());
    }

    void command_buffer::copy_buffer_to_image(image& dst, const buffer& src, VkImageLayout dest_final_layout, extent2 image_size, std::size_t buffer_offset, std::uint32_t image_idx, std::uint32_t image_count) const noexcept {
        VkBufferImageCopy copy_region{
            .bufferOffset = buffer_offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,

            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, image_idx, image_count},
            .imageOffset = {0, 0, 0},
            .imageExtent = {image_size.width(), image_size.height(), 1},
        };
        
        copy_buffer_to_image(dst, src, dest_final_layout, std::span<const VkBufferImageCopy, 1>(&copy_region, 1));
    }

    void command_buffer::copy_buffer_to_image(image& dst, const buffer& src, VkImageLayout dest_final_layout, std::span<const VkBufferImageCopy> copy_regions) const noexcept {
        VkBufferMemoryBarrier2 before_buffer_barrier {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = src,
            .offset = 0,
            .size = src.size(),
        };
        pipeline_barrier({}, std::span<VkBufferMemoryBarrier2, 1>{&before_buffer_barrier, 1}, {});
        image_transition(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst.layout(), dst.count());
        
        vkCmdCopyBufferToImage(handle, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_regions.size(), copy_regions.data());
        
        VkBufferMemoryBarrier2 after_buffer_barrier {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = src,
            .offset = 0,
            .size = src.size(),
        };
        pipeline_barrier({}, std::span<VkBufferMemoryBarrier2, 1>{&after_buffer_barrier, 1}, {});
        image_transition(dst, dest_final_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst.count());
    }
}