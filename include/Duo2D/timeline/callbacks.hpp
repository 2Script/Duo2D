#pragma once
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/error.hpp"
#include "Duo2D/core/render_process.fwd.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d::timeline {
	template<sl::size_t N, resource_table<N> Resources>
	struct callbacks {
		using render_process_type = render_process<N, Resources>;
		using result_type = result<void>;
	public:
		result_type(*on_frame_begin_fn)(render_process_type&) noexcept;
		result_type(*on_frame_end_fn)(render_process_type&) noexcept;

		result_type(*on_swap_chain_updated_fn)(render_process_type&) noexcept;
	};
}