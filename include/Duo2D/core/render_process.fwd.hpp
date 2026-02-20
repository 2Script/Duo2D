#pragma once
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/resource_table.hpp"


namespace d2d::impl {
	template<sl::size_t N, resource_table<N>, buffering_policy_t, typename>
	class device_allocation_group;

	template<sl::size_t N, resource_table<N>, typename>
	class render_process;
}

namespace d2d {
	template<sl::size_t N, resource_table<N> Resources>
	using render_process = impl::render_process<N, Resources, sl::index_sequence_of_length_type<buffering_policy::num_buffering_policies>>;
}
