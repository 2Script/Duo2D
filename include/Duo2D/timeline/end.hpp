#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d {
	template<command_family_t CommandFamily>
	struct end {};
}

namespace d2d::timeline {
	template<command_family_t CommandFamily>
	struct command<end<CommandFamily>> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources> const& proc, timeline::state<N, Resources>&, sl::empty_t) const noexcept {
			if constexpr(CommandFamily != command_family::graphics)
				if(proc.physical_device_ptr()->queue_family_infos[CommandFamily].index == proc.physical_device_ptr()->queue_family_infos[command_family::graphics].index)
					return {};
			
			vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[CommandFamily];

        	return cmd_buff.end();
		};
	};
}