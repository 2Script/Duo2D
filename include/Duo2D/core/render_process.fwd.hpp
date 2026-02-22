#pragma once
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/resource_table.hpp"


namespace d2d::impl {
	template<sl::size_t N, resource_table<N>, sl::size_t, buffering_policy_t, typename>
	class device_allocation_group;

	template<sl::size_t N, resource_table<N>, sl::size_t, typename>
	class render_process;
}

namespace d2d {
	template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
	using render_process = impl::render_process<N, Resources, CommandGroupCount, sl::index_sequence_of_length_type<buffering_policy::num_buffering_policies>>;
}
