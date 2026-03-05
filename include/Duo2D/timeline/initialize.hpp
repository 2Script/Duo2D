#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/window.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	template<command_family_t CommandFamily>
	struct initialize : timeline::event {
		constexpr static command_family_t family = CommandFamily;
	};
}

namespace d2d::timeline {
	template<command_family_t CommandFamily>
	struct command<initialize<CommandFamily>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT const& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			if constexpr(CommandFamily == command_family::present)
				if(!proc.has_dedicated_present_queue())
					return {};
			
			vk::command_buffer const& cmd_buff = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			RESULT_VERIFY(cmd_buff.reset());
        	return cmd_buff.begin(true);
		};
	};
}