#pragma once
#include "Duo2D/core/render_process.hpp"


namespace d2d {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	template<sl::size_t DstI, sl::size_t SrcI>
	constexpr result<void>    render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>::
	copy(
		allocation_segment_type<SrcI> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) & noexcept
	requires(
		!memory_policy::is_cpu_visible(allocation_segment_type<DstI>::config.memory)
	) {
		static_assert(
			memory_policy::is_cpu_writable(allocation_segment_type<SrcI>::config.memory) || 
			!memory_policy::is_cpu_visible(allocation_segment_type<SrcI>::config.memory),
			"Copy from cpu_local_gpu_write buffer to a gpu_local buffer is not allowed."
		);
		allocation_segment_type<DstI>& dst = static_cast<allocation_segment_type<DstI>&>(*this);

		if(dst_offset + size > dst.size_bytes() || src_offset + size > src.size_bytes()) 
			return errc::invalid_argument;

		//TODO: use next frame index if theres no garauntee current transfer command buffer is not in use
		constexpr sl::uint64_t timeout = sl::numeric_traits<sl::uint64_t>::max;

		const sl::index_t frame_idx = frame_index();
		vk::command_buffer const& transfer_command_buffer = command_buffers()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy];
		

		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, begin_dedicated_copy(timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout), pcwv_result);

		sl::array<1, VkBufferMemoryBarrier2> pre_copy_barriers{{
			VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
				.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
				.buffer = static_cast<VkBuffer>(src),
				.offset = src_offset,
				.size = size
			},
		}};
		transfer_command_buffer.pipeline_barrier({}, pre_copy_barriers, {});

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

		return end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout);
	}


	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	template<sl::size_t DstI, sl::size_t SrcI>
	constexpr result<void>    render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>::
	copy(
		allocation_segment_type<SrcI> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) & noexcept 
	requires(
		memory_policy::is_cpu_visible(allocation_segment_type<DstI>::config.memory) &&
		memory_policy::is_cpu_visible(allocation_segment_type<SrcI>::config.memory)
	) {
		static_assert(
			memory_policy::is_cpu_writable(allocation_segment_type<SrcI>::config.memory),
			"Copy from a cpu-visible buffer to a cpu_local_gpu_write buffer is not allowed."
		);
		allocation_segment_type<DstI>& dst = static_cast<allocation_segment_type<DstI>&>(*this);
		std::memcpy(dst.data() + dst_offset, src.data() + src_offset, size);

		return{};
	}
}

namespace d2d {
	template<sl::size_t DstI, sl::size_t SrcI, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void> copy(
		vk::device_allocation_segment<DstI, N, BufferConfigs, RenderProcessT>& dst,
		vk::device_allocation_segment<SrcI, N, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) noexcept {
		RenderProcessT& proc = static_cast<RenderProcessT&>(dst);
		return proc.template copy<DstI>(src, size, dst_offset, src_offset);
	}
}



namespace d2d{
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	constexpr result<sl::uint64_t>    render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>::
	begin_dedicated_copy(sl::index_t command_group_idx, sl::uint64_t timeout) & noexcept {
		const sl::index_t frame_idx = frame_index();
		vk::command_buffer const& transfer_command_buffer = command_buffers()[frame_idx][command_group_idx];

		const sl::uint64_t semaphore_pre_value = command_buffer_semaphore_values()[frame_idx][command_group_idx]++;
		const sl::uint64_t semaphore_post_value = semaphore_pre_value + 1;




		RESULT_VERIFY(command_buffer_semaphores()[frame_idx][command_group_idx].wait(semaphore_pre_value, timeout));
		
		RESULT_VERIFY(transfer_command_buffer.reset());
        RESULT_VERIFY(transfer_command_buffer.begin(true));

		return semaphore_post_value;
	}

	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	constexpr result<void>    render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>::
	end_dedicated_copy(sl::uint64_t wait_value, sl::index_t command_group_idx, sl::uint64_t timeout) const& noexcept {
		const sl::index_t frame_idx = frame_index();

		const vk::semaphore_submit_info semaphore_signal_info{
			command_buffer_semaphores()[frame_idx][command_group_idx],
			render_stage::group::all_transfer,
			wait_value,
		};

		vk::command_buffer const& transfer_command_buffer = command_buffers()[frame_idx][command_group_idx];
		RESULT_VERIFY(transfer_command_buffer.end());
		RESULT_VERIFY(transfer_command_buffer.submit(command_family::transfer, {}, {&semaphore_signal_info, 1}));


		RESULT_VERIFY(command_buffer_semaphores()[frame_idx][command_group_idx].wait(wait_value, timeout));

		return {};
	}
}