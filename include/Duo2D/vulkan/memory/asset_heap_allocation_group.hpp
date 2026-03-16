#pragma once

#include "Duo2D/vulkan/memory/asset_heap_allocation.hpp"

namespace d2d::vk::impl {
	template<sl::size_t N, auto AssetHeapConfigs, typename RenderProcessT, typename Seq>
	struct asset_heap_allocation_group;
}


namespace d2d::vk::impl {
	template<sl::size_t N, auto AssetHeapConfigs, typename RenderProcessT, sl::index_t... Is>
	struct asset_heap_allocation_group<N, AssetHeapConfigs, RenderProcessT, sl::index_sequence_type<Is...>> : 
		public asset_heap_allocation<
			N + Is, 
			sl::universal::get<sl::second_constant>(*std::next(AssetHeapConfigs.begin(), Is)),
			RenderProcessT
		>... 
	{
	protected:
		using asset_heap_allocation<
			N + Is, 
			sl::universal::get<sl::second_constant>(*std::next(AssetHeapConfigs.begin(), Is)),
			RenderProcessT
		>::realloc...;
	};
}