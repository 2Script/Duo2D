#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	template<render_stage_flags_t RenderStages>
	using wait_for = sl::constant_type<render_stage_flags_t, RenderStages>;


	template<render_stage_flags_t RenderStages>
	using signal_completion_at = sl::constant_type<render_stage_flags_t, RenderStages>;
}


namespace d2d::impl {
	template<command_family_t CommandFamily, typename CompletedAtT, typename WaitForT>
	struct submit_base : timeline::event {
		constexpr static bool ends_command_group = true;
		constexpr static command_family_t family = CommandFamily;
	};
}

namespace d2d {
	template<command_family_t CommandFamily, typename CompletedAtT = signal_completion_at<render_stage::none>, typename WaitForT = wait_for<render_stage::none>>
	struct submit : impl::submit_base<CommandFamily, CompletedAtT, WaitForT> {};
}


namespace d2d::timeline {
	template<command_family_t CommandFamily, render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<::d2d::impl::submit_base<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		result<void> operator()(render_process<N, Resources, CommandGroupCount>& proc, timeline::state<N, Resources, CommandGroupCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept;
	};
}


namespace d2d::timeline {
	template<command_family_t CommandFamily, render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<submit<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> :
		command<::d2d::impl::submit_base<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> {};


	template<render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<submit<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> :
		command<::d2d::impl::submit_base<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> 
	{
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		result<void> operator()(render_process<N, Resources, CommandGroupCount>& proc, timeline::state<N, Resources, CommandGroupCount>& timeline_state, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept;
	};
}

#include "Duo2D/timeline/submit.inl"