#pragma once
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	template<
		resource_key_t DstKey, resource_key_t SrcKey, 
		sl::size_t Size, 
		sl::uoffset_t DstOffset = 0, sl::uoffset_t SrcOffset = 0
	>
	struct copy_resource_data : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}


namespace d2d::timeline {
	template<resource_key_t DstKey, resource_key_t SrcKey, sl::size_t Size, sl::uoffset_t DstOffset, sl::uoffset_t SrcOffset>
	struct command<copy_resource_data<DstKey, SrcKey, Size, DstOffset, SrcOffset>> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(render_process<N, Resources, CommandGroupCount>& proc, timeline::state<N, Resources, CommandGroupCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			vk::command_buffer<N> const& transfer_command_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			transfer_command_buffer.copy(proc[DstKey], proc[SrcKey], Size, DstOffset, SrcOffset);
			return {};
		};
	};
}