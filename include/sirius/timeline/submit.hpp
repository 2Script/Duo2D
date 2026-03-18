#pragma once 
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<render_stage_flags_t RenderStages>
	using wait_for = sl::constant_type<render_stage_flags_t, RenderStages>;


	template<render_stage_flags_t RenderStages>
	using signal_completion_at = sl::constant_type<render_stage_flags_t, RenderStages>;
}


namespace acma::impl {
	template<command_family_t CommandFamily, typename CompletedAtT, typename WaitForT>
	struct submit_base : timeline::event {
		constexpr static bool ends_command_group = true;
		constexpr static command_family_t family = CommandFamily;
	};
}

namespace acma {
	template<command_family_t CommandFamily, typename CompletedAtT = signal_completion_at<render_stage::group::all>, typename WaitForT = wait_for<render_stage::none>>
	struct submit : impl::submit_base<CommandFamily, CompletedAtT, WaitForT> {};
}


namespace acma::timeline {
	template<command_family_t CommandFamily, render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<::acma::impl::submit_base<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		result<void> operator()(RenderProcessT& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept;
	};
}


namespace acma::timeline {
	template<command_family_t CommandFamily, render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<submit<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> :
		command<::acma::impl::submit_base<CommandFamily, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> {};


	template<render_stage_flags_t CompleteStages, render_stage_flags_t WaitStages>
	struct command<submit<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> :
		command<::acma::impl::submit_base<command_family::present, signal_completion_at<CompleteStages>, wait_for<WaitStages>>> 
	{
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		result<void> operator()(RenderProcessT& proc, window&, timeline::state& timeline_state, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept;
	};
}

#include "sirius/timeline/submit.inl"