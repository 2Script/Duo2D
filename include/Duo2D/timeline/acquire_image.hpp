#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/core/invoke_all.def.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	struct acquire_image : timeline::event {
		constexpr static bool ends_command_group = true;
	};
}

namespace d2d::timeline {
	template<>
	struct command<acquire_image> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
		constexpr result<void> operator()(render_process<N, Resources, CommandGroupCount>& proc, timeline::state<N, Resources, CommandGroupCount>& timeline_state, sl::empty_t, auto) const noexcept {
			RESULT_TRY_COPY_UNSCOPED(bool swap_chain_updated, proc.verify_swap_chain(
        	    vkAcquireNextImageKHR(*proc.logical_device_ptr(), proc.swap_chain(), UINT64_MAX, proc.acquisition_semaphores()[proc.frame_index()], VK_NULL_HANDLE, &timeline_state.image_index), false
        	), sc);
			if(swap_chain_updated)
				D2D_INVOKE_ALL(proc.timeline_callbacks(), on_swap_chain_updated_fn, proc);
			return {};
		};
	};
}