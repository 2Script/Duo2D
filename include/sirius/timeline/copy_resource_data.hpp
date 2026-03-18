#pragma once
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/setup.hpp"
#include "sirius/core/window.hpp"
#include "sirius/vulkan/memory/pipeline.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<
		buffer_key_t DstKey, buffer_key_t SrcKey, 
		sl::size_t Size, 
		sl::uoffset_t DstOffset = 0, sl::uoffset_t SrcOffset = 0
	>
	struct copy_buffer_data : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}


namespace acma::timeline {
	template<buffer_key_t DstKey, buffer_key_t SrcKey, sl::size_t Size, sl::uoffset_t DstOffset, sl::uoffset_t SrcOffset>
	struct command<copy_buffer_data<DstKey, SrcKey, Size, DstOffset, SrcOffset>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			vk::command_buffer const& transfer_command_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			transfer_command_buffer.copy(proc[DstKey], proc[SrcKey], Size, DstOffset, SrcOffset);
			return {};
		};
	};
}