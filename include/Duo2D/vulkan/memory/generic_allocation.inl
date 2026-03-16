#pragma once
#include "Duo2D/vulkan/memory/generic_allocation.hpp"
#include <streamline/universal/make.hpp>


namespace d2d::vk {
	template<coupling_policy_t CouplingPolicy, typename RenderProcessT>
	result<void>    generic_allocation<CouplingPolicy, RenderProcessT>::
	initialize(
		std::shared_ptr<logical_device> logi_device, 
		std::shared_ptr<physical_device> phys_device//,
		//std::shared_ptr<command_pool> transfer_command_pool
	) noexcept {
        mems = sl::universal::make<sl::array<allocation_count, memory_ptr_type>>(
			logi_device, 
			sl::in_place_repeat_tag<allocation_count>, 
			[](std::shared_ptr<logical_device>& logi_device_ptr, auto) noexcept {
				return memory_ptr_type(logi_device_ptr);
			}
		);
		logi_device_ptr = logi_device;
		phys_device_ptr = phys_device;


		//for(sl::index_t i = 0; i < allocation_count; ++i) {
		//	RESULT_TRY_MOVE(realloc_command_buffers[i], make<command_buffer>(logi_device, phys_device, transfer_command_pool))
		//	RESULT_TRY_MOVE(realloc_fences[i], make<fence>(logi_device));
		//}
		
        return {};
    }
}
