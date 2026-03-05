#pragma once
#include "Duo2D/core/render_process.fwd.hpp"

#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "Duo2D/core/frames_in_flight.def.hpp"
#include "Duo2D/vulkan/memory/device_allocation.hpp"
#include "Duo2D/vulkan/memory/device_allocation_segment.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"



namespace d2d::vk::impl {
	template<sl::size_t N, buffer_config_table<N>, typename, coupling_policy_t, typename>
	class device_allocations_grouped_by_buffer_policy;

	template<sl::size_t N, buffer_config_table<N>, typename, typename>
	class device_allocation_group;
}

namespace d2d::vk::impl {
	template<sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT, coupling_policy_t CP, memory_policy_t MP>
	using device_allocation_filter_sequence = sl::filtered_sequence_t<
		sl::index_sequence_of_length_type<N>,
		[]<sl::index_t I>(sl::index_constant_type<I>){
			return 
				vk::device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory == MP &&
				vk::device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.coupling == CP;
		}
	>;
}


namespace d2d::vk::impl {
	template<sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT, coupling_policy_t CouplingPolicy, memory_policy_t... MemoryPolicyIs>
	class device_allocations_grouped_by_buffer_policy<N, BufferConfigs, RenderProcessT, CouplingPolicy, sl::index_sequence_type<MemoryPolicyIs...>> : 
		public device_allocation<
			D2D_FRAMES_IN_FLIGHT, 
			device_allocation_filter_sequence<N, BufferConfigs, RenderProcessT, CouplingPolicy, MemoryPolicyIs>,
			CouplingPolicy, MemoryPolicyIs,
			N, BufferConfigs,
			RenderProcessT
		>... 
	{
	protected:
		using device_allocation<
			D2D_FRAMES_IN_FLIGHT, 
			device_allocation_filter_sequence<N, BufferConfigs, RenderProcessT, CouplingPolicy, MemoryPolicyIs>,
			CouplingPolicy, MemoryPolicyIs,
			N, BufferConfigs,
			RenderProcessT
		>::realloc...; 
	};

	template<sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT, coupling_policy_t... CouplingPolicyIs>
	class device_allocation_group<N, BufferConfigs, RenderProcessT, sl::index_sequence_type<CouplingPolicyIs...>> : 
		public device_allocations_grouped_by_buffer_policy<
			N, BufferConfigs, RenderProcessT, CouplingPolicyIs, sl::index_sequence_of_length_type<memory_policy::num_memory_policies>
		>... 
	{
		template<sl::index_t, sl::size_t _N, buffer_config_table<_N>, typename>
		friend class vk::impl::device_allocation_segment_base;
	private:
		using device_allocations_grouped_by_buffer_policy<
			N, BufferConfigs, RenderProcessT, CouplingPolicyIs, sl::index_sequence_of_length_type<memory_policy::num_memory_policies>
		>::realloc...;
	};
}
