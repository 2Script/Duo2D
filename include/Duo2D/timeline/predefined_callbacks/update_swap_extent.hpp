#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/universal/get.hpp>

#include "Duo2D/arith/size.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d::timeline::predefined_callbacks {
	template<typename WindowT, resource_key_t SwapExtentResourceKey, sl::uoffset_t ResourceOffsetBytes = 0>
	result<void> update_swap_extent(typename WindowT::render_process_type& proc) noexcept {
		std::memcpy(
			sl::universal::get<SwapExtentResourceKey>(proc).data() + ResourceOffsetBytes, 
			&proc.swap_chain().extent(), 
			sizeof(extent2)
		);
		return {};
	}
}