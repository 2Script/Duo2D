#pragma once
#include <streamline/functional/functor/invoke_each.hpp>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/core/command_buffer.hpp"


namespace d2d::vk {
    result<command_buffer> command_buffer::create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> pool) noexcept {
        command_buffer ret{};
        ret.logi_device_ptr = logi_device;
		ret.phys_device_ptr = phys_device;
        ret.cmd_pool_ptr = pool;

        VkCommandBufferAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = *pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        __D2D_VULKAN_VERIFY(vkAllocateCommandBuffers(*logi_device, &alloc_info, &ret.handle));
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

    result<void> command_buffer::submit(command_family_t family, std::span<const semaphore_submit_info> wait_semaphore_infos, std::span<const semaphore_submit_info> signal_semaphore_infos, VkFence out_fence) const noexcept {
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

    result<void> command_buffer::wait(command_family_t family) const noexcept {
        __D2D_VULKAN_VERIFY(vkQueueWaitIdle(logi_device_ptr->queues[family]));
        return {};
    }

    void command_buffer::free() const noexcept {
        return vkFreeCommandBuffers(*logi_device_ptr, *cmd_pool_ptr, 1, &handle);
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

    void command_buffer::image_transition(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t) const noexcept {
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
                .layerCount = 1,
            },
        };

        pipeline_barrier({}, {}, std::span<const VkImageMemoryBarrier2, 1>{&image_barrier, 1});
        img.layout() = new_layout;
    }
}

namespace d2d::vk {
    void command_buffer::begin_draw(
		std::span<const VkRenderingAttachmentInfo> color_attachments, 
		VkRenderingAttachmentInfo const& depth_attachment, 
		rect<std::uint32_t> render_area_bounds,
		rect<float> viewport_bounds,
		rect<std::uint32_t> scissor_bounds
	) const noexcept {
        //Set render pass
		VkRenderingInfo rendering_info{
		    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		    .renderArea{
				.offset = static_cast<VkOffset2D>(render_area_bounds.pos),
				.extent = static_cast<VkExtent2D>(render_area_bounds.size),
			},
		    .layerCount = 1,
		    .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
		    .pColorAttachments = color_attachments.data(),
		    .pDepthAttachment = &depth_attachment
		};
		vkCmdBeginRendering(handle, &rendering_info);

        //Set viewport
        VkViewport v{viewport_bounds.x(), viewport_bounds.y(), viewport_bounds.width(), viewport_bounds.height(), 0.f, 1.f};
        vkCmdSetViewport(handle, 0, 1, &v);

        //Set viewport crop
        VkRect2D scissor = static_cast<VkRect2D>(scissor_bounds);
        vkCmdSetScissor(handle, 0, 1, &scissor);
    }


    void command_buffer::end_draw() const noexcept {
        vkCmdEndRendering(handle);
    }
}


namespace d2d::vk {
	template<typename T, sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, auto AssetHeapConfigs, typename RenderProcessT>
	void command_buffer::bind_buffer(device_allocation_segment<I, N, BufferConfigs, RenderProcessT> const& buff, pipeline_layout<shader_stage::all_graphics, T, BufferConfigs, AssetHeapConfigs> const& layout) const noexcept {
		constexpr buffer_config config = device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config;
		
		if constexpr(config.usage & buffer_usage_policy::index)
            vkCmdBindIndexBuffer(handle, buff.buffs[buff.current_buffer_index()], 0, T::index_type);

		if constexpr(config.usage & buffer_usage_policy::push_constant)
			vkCmdPushConstants(handle, layout, config.stages, 0, config.initial_capacity_bytes, buff.bytes.data());
	}

	template<typename T, sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, auto AssetHeapConfigs, typename RenderProcessT>
	void command_buffer::bind_buffer(device_allocation_segment<I, N, BufferConfigs, RenderProcessT> const& buff, pipeline_layout<shader_stage::compute, T, BufferConfigs, AssetHeapConfigs> const& layout) const noexcept {
		constexpr buffer_config config = device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config;

		if constexpr(config.usage & buffer_usage_policy::push_constant)
			vkCmdPushConstants(handle, layout, config.stages, 0, config.initial_capacity_bytes, buff.bytes.data());
	}

    template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
	void command_buffer::bind_pipeline(pipeline<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& p) const noexcept {
        vkCmdBindPipeline(handle, static_cast<VkPipelineBindPoint>(BindPoint), p);
	}

	
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
    void command_buffer::draw(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_command_buffer_offsets, 
		sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_count_buffer_offsets 
	) const noexcept {
		constexpr auto draw_command = []<sl::index_t I>(
			VkCommandBuffer cmd_buff,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_cmd_buff_offsets, 
			sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_cnt_buff_offsets,
			sl::index_constant_type<I>
		) noexcept -> void {
			const sl::uoffset_t draw_cmd_offset = draw_cmd_buff_offsets[I];
			const sl::uoffset_t draw_cnt_offset = draw_cnt_buff_offsets[I];
			auto const& draw_cmd_buff = sl::universal::get<T::draw_buffers[I].key>(proc);
			auto const& draw_cnt_buff = sl::universal::get<T::draw_buffers[I].value>(proc);

        	if constexpr (requires { T::index_type; }) {
				constexpr static sl::size_t stride = sizeof(indexed_draw_command_t);
				const sl::uint32_t final_max_draw_count = std::min(T::max_draw_count(), static_cast<sl::uint32_t>((draw_cmd_buff.capacity_bytes() - stride - draw_cmd_offset)/stride) + 1);
        	    vkCmdDrawIndexedIndirectCount(cmd_buff, static_cast<VkBuffer>(draw_cmd_buff), draw_cmd_offset, static_cast<VkBuffer>(draw_cnt_buff), draw_cnt_offset, final_max_draw_count, stride);
			} else {
				constexpr static sl::size_t stride = sizeof(draw_command_t);
				const sl::uint32_t final_max_draw_count = std::min(T::max_draw_count(), static_cast<sl::uint32_t>((draw_cmd_buff.capacity_bytes() - stride - draw_cmd_offset)/stride) + 1);
        	    vkCmdDrawIndirectCount(cmd_buff, static_cast<VkBuffer>(draw_cmd_buff), draw_cmd_offset, static_cast<VkBuffer>(draw_cnt_buff), draw_cnt_offset, final_max_draw_count, stride);
			}
		};

		return sl::functor::invoke_each<draw_command>{}(sl::index_sequence_of_length<decltype(T::draw_buffers)::size()>, *this, render_proc, draw_command_buffer_offsets, draw_count_buffer_offsets);
    }
	
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
    void command_buffer::dispatch(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		sl::array<decltype(T::dispatch_buffers)::size(), sl::uoffset_t> buffer_offsets
	) const noexcept {
		constexpr auto dispatch_command = []<sl::index_t I>(
			VkCommandBuffer cmd_buff,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			sl::array<decltype(T::dispatch_buffers)::size(), sl::uoffset_t> buff_offsets, 
			sl::index_constant_type<I>
		) noexcept -> void {
			vkCmdDispatchIndirect(cmd_buff, static_cast<VkBuffer>(sl::universal::get<T::dispatch_buffers[I]>(proc)), buff_offsets[I]);
		};

		return sl::functor::invoke_each<dispatch_command>{}(sl::index_sequence_of_length<decltype(T::dispatch_buffers)::size()>, *this, render_proc, buffer_offsets);
    }
}



namespace d2d::vk {
	template<sl::index_t I, sl::index_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    void command_buffer::copy(
		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst, 
		device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src, 
		std::span<const VkBufferCopy> copy_regions
	) const noexcept {
        vkCmdCopyBuffer(handle, src.buffs[src.current_buffer_index()], dst.buffs[dst.current_buffer_index()], copy_regions.size(), copy_regions.data());
    }


	template<sl::index_t I, sl::index_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    void command_buffer::copy(
		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst, 
		device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src, 
		std::size_t size, 
		sl::uoffset_t dst_offset, 
		sl::uoffset_t src_offset
	) const noexcept {
        VkBufferCopy copy_region{ 
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size = size,
        };
        return copy(dst, src, {&copy_region, 1});
    }
}