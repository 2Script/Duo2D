#pragma once
#include "Duo2D/core/render_process.hpp"

#include "Duo2D/timeline/dedicated_command_group.hpp"


namespace d2d {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	template<sl::size_t I, sl::size_t J>
	constexpr result<void>    render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>::
	copy(
		allocation_segment_type<J> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) & noexcept {
		allocation_segment_type<I>& dst = static_cast<allocation_segment_type<I>&>(*this);
		//TODO: use next frame index if theres no garauntee current transfer command buffer is not in use
		const sl::index_t frame_idx = frame_index();
		vk::command_buffer const& transfer_command_buffer = command_buffers()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy];
		const sl::uint64_t semaphore_value = command_buffer_semaphore_values()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy]++;

		VkSemaphoreWaitInfo semaphore_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &command_buffer_semaphores()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy],
			.pValues = &semaphore_value
		};
		vk::semaphore_submit_info semaphore_signal_info{
			command_buffer_semaphores()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy],
			render_stage::group::all_transfer,
			semaphore_value + 1,
		};

		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*logi_device_ptr, &semaphore_wait_info, std::numeric_limits<sl::uint64_t>::max()));
		RESULT_VERIFY(transfer_command_buffer.reset());
        RESULT_VERIFY(transfer_command_buffer.begin(true));
		transfer_command_buffer.copy(dst, src, size, dst_offset, src_offset);

		sl::array<2, VkBufferMemoryBarrier2> post_copy_barriers{{
			VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.buffer = static_cast<VkBuffer>(src),
				.offset = src_offset,
				.size = size
			},
			VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.buffer = static_cast<VkBuffer>(dst),
				.offset = dst_offset,
				.size = size
			},
		}};
		transfer_command_buffer.pipeline_barrier({}, post_copy_barriers, {});
		RESULT_VERIFY(transfer_command_buffer.end());
		RESULT_VERIFY(transfer_command_buffer.submit(command_family::transfer, {}, {&semaphore_signal_info, 1}));
		return {};
	}
}

namespace d2d {
	template<sl::size_t I, sl::size_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void> copy(
		vk::device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst,
		vk::device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) noexcept {
		RenderProcessT& proc = static_cast<RenderProcessT&>(dst);
		return proc.template copy<I>(src, size, dst_offset, src_offset);
	}
}

namespace d2d{
	
}