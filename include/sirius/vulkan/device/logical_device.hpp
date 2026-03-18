#pragma once
#include <memory>
#include <streamline/containers/tuple.hpp>

#include <vulkan/vulkan_core.h>

#include "sirius/core/command_family.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/traits/vk_traits.hpp"


__D2D_DECLARE_VK_TRAITS(VkDevice);


#define __D2D_REMOVE_FIRST_INDIRECT(...) __D2D_REMOVE_FIRST(__VA_ARGS__)
#define __D2D_REMOVE_FIRST(A, ...) __VA_ARGS__
#define __D2D_VK_EXT_FNS \
EXT_FN(vkCmdPushDescriptorSet, KHR)
//EXT_FN(vkCmdPushData, EXT) \
//EXT_FN(vkWriteSamplerDescriptors, EXT) \
//EXT_FN(vkWriteResourceDescriptors, EXT) \
//

namespace acma::vk {
	namespace extended_function {
	enum {
		#define EXT_FN(name, suffix) name,
		__D2D_VK_EXT_FNS
		#undef EXT_FN

		num_extended_functions
	};
	}
}


namespace acma::vk {
    struct logical_device : vulkan_ptr<VkDevice, vkDestroyDevice> {
		using vulkan_functions_type = sl::tuple<
			#define EXT_FN(name, suffix) ,PFN_##name##suffix
			__D2D_REMOVE_FIRST_INDIRECT(__D2D_VK_EXT_FNS)
			#undef EXT_FN
		>;

	public:
        static result<logical_device> create(std::weak_ptr<physical_device> associated_phys_device, bool create_present_queue) noexcept;

	public:
		constexpr vulkan_functions_type const& vulkan_functions() const& noexcept { return vulkan_fns; }

		
	private:
		vulkan_functions_type vulkan_fns;
    public:
        //May need to be per-window instead of per-device?
        std::array<VkQueue, command_family::num_families> queues;
    };
}
