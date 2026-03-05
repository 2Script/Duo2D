#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/window.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"
#include "Duo2D/core/invoke_all.def.hpp"
#include "Duo2D/timeline/event.hpp"
#include "Duo2D/timeline/callback_event.hpp"


namespace d2d {
	struct acquire_image : timeline::event {
		constexpr static bool ends_command_group = true;
	};
}

namespace d2d::timeline {
	template<>
	struct command<acquire_image> {
		template<typename RenderProcessT>
		constexpr result<void> operator()(RenderProcessT& proc, window& win, timeline::state& timeline_state, sl::empty_t, auto) const noexcept {
			RESULT_TRY_COPY_UNSCOPED(bool swap_chain_updated, win.verify_swap_chain(
        	    vkAcquireNextImageKHR(
					*proc.logical_device_ptr(),
					proc.swap_chain(),
					UINT64_MAX,
					proc.acquisition_semaphores()[proc.frame_index()],
					VK_NULL_HANDLE,
					&timeline_state.image_index
				),
				proc.logical_device_ptr(),
				proc.physical_device_ptr(),
				false
        	), sc);
			if(swap_chain_updated)
				D2D_INVOKE_ALL(proc.timeline_callbacks(), on_swap_chain_updated, proc, win, timeline_state);
			return {};
		};
	};
}