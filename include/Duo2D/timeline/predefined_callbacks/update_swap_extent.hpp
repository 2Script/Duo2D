#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/universal/get.hpp>

#include "Duo2D/arith/size.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/buffer_config_table.hpp"


namespace d2d::timeline::predefined_callbacks {
	template<typename WindowT, buffer_key_t SwapExtentBufferKey, sl::uoffset_t BufferOffsetBytes = 0>
	result<void> update_swap_extent(typename WindowT::render_process_type& proc) noexcept {
		std::memcpy(
			sl::universal::get<SwapExtentBufferKey>(proc).data() + BufferOffsetBytes, 
			&proc.swap_chain().extent(), 
			sizeof(extent2)
		);
		return {};
	}
}