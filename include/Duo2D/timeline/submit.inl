#pragma once 
#include "Duo2D/timeline/submit.hpp"
#include "Duo2D/core/invoke_all.def.hpp"

#include <vulkan/vulkan.h>


namespace d2d::timeline::impl {
	template<command_family_t CommandFamily>
	struct extra_semaphores {
		constexpr static sl::size_t wait_count = 0;
		constexpr static sl::size_t signal_count = 0;

		constexpr static std::array<vk::semaphore_submit_info, wait_count> 
		wait(auto const&, auto&) noexcept { return {}; }

		constexpr static std::array<vk::semaphore_submit_info, signal_count> 
		signal(auto const&, auto&) noexcept { return {}; }
	};
}

namespace d2d::timeline::impl {
	template<> struct extra_semaphores<command_family::graphics> {
		constexpr static sl::size_t wait_count = 1;
		constexpr static sl::size_t signal_count = 1;

		constexpr static std::array<vk::semaphore_submit_info, wait_count>
		wait(auto const& proc, auto&) noexcept { 
			return {{{proc.acquisition_semaphores()[proc.frame_index()], render_stage::color_attachment_output}}}; 
		}

		constexpr static std::array<vk::semaphore_submit_info, signal_count>
		signal(auto const& proc, auto& timeline_state) noexcept { 
			return {{{proc.graphics_semaphores()[timeline_state.image_index], render_stage::color_attachment_output}}}; 
		}
	};

	template<> struct extra_semaphores<command_family::present> {
		constexpr static sl::size_t wait_count = 1;
		constexpr static sl::size_t signal_count = 1;

		constexpr static std::array<vk::semaphore_submit_info, wait_count>
		wait(auto const& proc, auto& timeline_state) noexcept { 
			return {{{proc.graphics_semaphores()[timeline_state.image_index], render_stage::group::all}}}; 
		}

		constexpr static std::array<vk::semaphore_submit_info, signal_count>
		signal(auto const& proc, auto& timeline_state) noexcept { 
			return {{{proc.pre_present_semaphores()[timeline_state.image_index], render_stage::group::all}}}; 
		}
	};
}



namespace d2d::timeline {
	template<command_family_t CommandFamily, render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
	result<void> command<::d2d::impl::submit_base<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>>::operator()(
		render_process<N, Resources, CommandGroupCount>& proc, 
		timeline::state<N, Resources, CommandGroupCount>& timeline_state, 
		sl::empty_t,
		sl::index_constant_type<CommandGroupIdx>
	) const noexcept {
		const sl::index_t frame_idx = proc.frame_index();

		constexpr sl::size_t extra_wait_semaphore_count = impl::extra_semaphores<CommandFamily>::wait_count;
		constexpr sl::size_t max_wait_semaphore_count = command_family::num_families + extra_wait_semaphore_count;

		std::array<vk::semaphore_submit_info, max_wait_semaphore_count> wait_semaphore_infos;
		sl::size_t wait_seamphore_count = 0;

		for(command_family_t cf = 0; cf < command_family::num_families; ++cf)
			if(::d2d::impl::has_command_family(cf, WaitStages))
				wait_semaphore_infos[wait_seamphore_count++] = {proc.command_family_semaphores()[frame_idx][cf], d2d::render_stage::group::all/*::d2d::impl::filter_by_command_family(cf, WaitStages)*/, proc.command_family_semaphore_values()[frame_idx][cf]};
		
		for(sl::index_t i = 0; i < extra_wait_semaphore_count; ++i)
			wait_semaphore_infos[wait_seamphore_count++] = impl::extra_semaphores<CommandFamily>::wait(proc, timeline_state)[i];


		constexpr sl::size_t extra_signal_semaphore_count = impl::extra_semaphores<CommandFamily>::signal_count;
		constexpr sl::size_t max_signal_semaphore_count = 2 + extra_signal_semaphore_count;

		std::array<vk::semaphore_submit_info, max_signal_semaphore_count> signal_semaphore_infos;
		sl::size_t signal_semaphore_count = 2;
		signal_semaphore_infos[0] = {proc.command_family_semaphores()[frame_idx][CommandFamily], CompleteStages, ++proc.command_family_semaphore_values()[frame_idx][CommandFamily]};
		signal_semaphore_infos[1] = {proc.command_buffer_semaphores()[frame_idx][CommandGroupIdx], CompleteStages, ++proc.command_buffer_semaphore_values()[frame_idx][CommandGroupIdx]};
		
		for(sl::index_t i = 0; i < extra_signal_semaphore_count; ++i)
			signal_semaphore_infos[signal_semaphore_count++] = impl::extra_semaphores<CommandFamily>::signal(proc, timeline_state)[i];


		
		vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[frame_idx][CommandGroupIdx];
		RESULT_VERIFY(cmd_buff.end());
		return cmd_buff.submit(
			CommandFamily,
			{wait_semaphore_infos.data(), wait_seamphore_count},
			{signal_semaphore_infos.data(), signal_semaphore_count}//,
			//CommandFamily == command_family::graphics ? static_cast<VkFence>(proc.graphics_fences()[frame_idx]) : VK_NULL_HANDLE
		);
	}
}


namespace d2d::timeline {
	template<render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
	result<void> command<submit<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>>::operator()(
		render_process<N, Resources, CommandGroupCount>& proc, 
		timeline::state<N, Resources, CommandGroupCount>& timeline_state,
		sl::empty_t,
		sl::index_constant_type<CommandGroupIdx>
	) const noexcept {
		if(proc.has_dedicated_present_queue()) {
			vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];


			VkImageMemoryBarrier2 pre_present_barrier {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = proc.physical_device_ptr()->queue_family_infos[command_family::graphics].index,
				.dstQueueFamilyIndex = proc.physical_device_ptr()->queue_family_infos[command_family::present].index,
				.image = proc.swap_chain().images()[timeline_state.image_index],
				.subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
			};

			cmd_buff.pipeline_barrier({}, {}, {&pre_present_barrier, 1});

			
			//Call base submit function
			RESULT_VERIFY((command<::d2d::impl::submit_base<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>>::
			operator()(proc, timeline_state, sl::empty_t{}, sl::index_constant<CommandGroupIdx>)));
		} else {
		const sl::index_t frame_idx = proc.frame_index();
			VkSemaphoreWaitInfo wait_info{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
				.flags = 0,
				.semaphoreCount = 1,
				.pSemaphores = &proc.command_family_semaphores()[frame_idx][command_family::present],
				.pValues = &proc.command_family_semaphore_values()[frame_idx][command_family::present],
			};
			__D2D_VULKAN_VERIFY(vkWaitSemaphores(*proc.logical_device_ptr(), &wait_info, std::numeric_limits<sl::uint64_t>::max()));

			VkSemaphoreSignalInfo family_signal_info{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
				.semaphore = proc.command_family_semaphores()[frame_idx][command_family::present],
				.value = ++proc.command_family_semaphore_values()[frame_idx][command_family::present],
			};
			__D2D_VULKAN_VERIFY(vkSignalSemaphore(*proc.logical_device_ptr(), &family_signal_info));

			VkSemaphoreSignalInfo buffer_signal_info{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
				.semaphore = proc.command_buffer_semaphores()[frame_idx][CommandGroupIdx],
				.value = ++proc.command_buffer_semaphore_values()[frame_idx][CommandGroupIdx],
			};
			__D2D_VULKAN_VERIFY(vkSignalSemaphore(*proc.logical_device_ptr(), &buffer_signal_info));
		}

		VkPresentInfoKHR present_info{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = proc.has_dedicated_present_queue() ? 
				&proc.pre_present_semaphores()[timeline_state.image_index] : 
				&proc.graphics_semaphores()[timeline_state.image_index],
			.swapchainCount = 1,
			.pSwapchains = &proc.swap_chain(),
			.pImageIndices = &timeline_state.image_index,
		};
		RESULT_TRY_COPY_UNSCOPED(bool swap_chain_updated, proc.verify_swap_chain(vkQueuePresentKHR(proc.logical_device_ptr()->queues[command_family::present], &present_info), true), sc);
		if(swap_chain_updated)
			D2D_INVOKE_ALL(proc.timeline_callbacks(), on_swap_chain_updated_fn, proc);
		return {};
	}
}