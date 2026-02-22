#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"
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
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(render_process<N, Resources, CommandGroupCount> const& proc, timeline::state<N, Resources, CommandGroupCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			if constexpr(CommandFamily == command_family::present)
				if(!proc.has_dedicated_present_queue())
					return {};
			
			vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			RESULT_VERIFY(cmd_buff.reset());
        	return cmd_buff.begin(true);
		};
	};
}