#pragma once
#include <vulkan/vulkan.h>

#include "Duo2D/core/coupling_policy.hpp"
#include "Duo2D/core/frames_in_flight.def.hpp"
#include "Duo2D/traits/vk_traits.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"



__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);


namespace d2d::vk::impl {
	constexpr sl::array<coupling_policy::num_coupling_policies, sl::size_t> allocation_counts{{D2D_FRAMES_IN_FLIGHT, 1}};
}


namespace d2d::vk {	
	template<coupling_policy_t, typename>
	class generic_allocation;
}
