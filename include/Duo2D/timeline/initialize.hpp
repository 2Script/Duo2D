#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d {
	template<command_family_t CommandFamily>
	struct initialize {};
}

namespace d2d::timeline {
	template<command_family_t CommandFamily>
	struct command<initialize<CommandFamily>> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources> const& proc, timeline::state<N, Resources>& state, sl::empty_t) const noexcept {
			if constexpr(CommandFamily == command_family::present)
				if(!proc.has_dedicated_present_queue())
					return {};
			
			vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[proc.frame_index()][CommandFamily];
			
			state.current_command_buffer_semaphore_values[CommandFamily] = proc.command_buffer_semaphore_values()[proc.frame_index()][CommandFamily]->fetch_add(1, std::memory_order::relaxed);

			VkSemaphoreWaitInfo semaphore_wait_info{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
				.flags = 0,
				.semaphoreCount = 1,
				.pSemaphores = &proc.command_buffer_semaphores()[proc.frame_index()][CommandFamily],
				.pValues = &state.current_command_buffer_semaphore_values[CommandFamily]
			};

			__D2D_VULKAN_VERIFY(vkWaitSemaphores(*proc.logical_device_ptr(), &semaphore_wait_info, std::numeric_limits<sl::uint64_t>::max()));
			RESULT_VERIFY(cmd_buff.reset());
        	return cmd_buff.begin(true);
		};
	};
}