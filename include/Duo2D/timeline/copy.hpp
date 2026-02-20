#pragma once
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"


namespace d2d {
	template<typename T>
	struct commit_transfers {};
}


namespace d2d::timeline {
	template<resource_key_t... Keys>
	struct command<commit_transfers<resource_key_sequence_type<Keys...>>> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources>& proc, timeline::state<N, Resources>&, sl::empty_t) const noexcept {
			return proc.template commit_transfers<Keys...>();
		};
	};
}