#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/universal/get.hpp>

#include "Duo2D/arith/size.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"


namespace d2d::timeline::predefined_callbacks {
	template<typename InstanceT, buffer_key_t SwapExtentBufferKey, sl::uoffset_t BufferOffsetBytes = 0>
	result<void> update_swap_extent(typename InstanceT::render_process_type& proc, typename InstanceT::window_type& win, auto&) noexcept {
		std::memcpy(
			sl::universal::get<SwapExtentBufferKey>(proc).data() + BufferOffsetBytes, 
			&win.swap_chain().extent(), 
			sizeof(extent2)
		);
		return {};
	}
}