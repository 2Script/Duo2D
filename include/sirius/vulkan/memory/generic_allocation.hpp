#pragma once
#include "sirius/vulkan/memory/generic_allocation.fwd.hpp"

#include "sirius/core/coupling_policy.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"


namespace acma::vk {
	template<coupling_policy_t CouplingPolicy, typename RenderProcessT>
	class generic_allocation {
	public:
		constexpr static sl::size_t allocation_count = impl::allocation_counts[CouplingPolicy];
		using memory_ptr_type = vulkan_ptr<VkDeviceMemory, vkFreeMemory>;


	protected:
		result<void> initialize(
			std::shared_ptr<logical_device> logi_device, 
			physical_device* phys_device//,
			//std::shared_ptr<command_pool> transfer_command_pool
		) noexcept;


	protected:
		sl::array<allocation_count, memory_ptr_type> mems;
	protected:
		std::shared_ptr<logical_device> logi_device_ptr;
		physical_device* phys_device_ptr;
	};
}

#include "sirius/vulkan/memory/generic_allocation.inl"