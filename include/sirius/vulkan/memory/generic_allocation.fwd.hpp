#pragma once
#include <vulkan/vulkan.h>

#include "sirius/core/coupling_policy.hpp"
#include "sirius/core/frames_in_flight.def.hpp"
#include "sirius/traits/vk_traits.hpp"
#include "sirius/vulkan/device/logical_device.hpp"



__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);


namespace acma::vk::impl {
	constexpr sl::array<coupling_policy::num_coupling_policies, sl::size_t> allocation_counts{{D2D_FRAMES_IN_FLIGHT, 1}};
}


namespace acma::vk {	
	template<coupling_policy_t, typename>
	class generic_allocation;
}
