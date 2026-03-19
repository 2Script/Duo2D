#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include <vulkan/vulkan_core.h>


namespace acma::vk {
    result<logical_device> logical_device::create(physical_device* associated_phys_device, bool windowing) noexcept {
		const sl::size_t queue_create_info_count = command_family::num_distinct_families + static_cast<sl::size_t>(windowing);

        //Create desired queues for each queue family
        constexpr static float priority = 1.f;
        std::array<VkDeviceQueueCreateInfo, command_family::num_families> queue_create_infos{};
		std::unordered_map<sl::uint32_t, bool> duplicate_index{};
		sl::uint32_t queue_create_count = 0;
        for(std::size_t i = 0; i < queue_create_info_count; ++i) {
			const sl::uint32_t index = associated_phys_device->queue_family_infos[i].index;
			if(duplicate_index[index])
				continue;
            VkDeviceQueueCreateInfo queue_create_info{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = index,
                .queueCount = 1,
                .pQueuePriorities = &priority,
            };
			duplicate_index[index] = true;
            queue_create_infos[i] = queue_create_info;
			++queue_create_count;
        }

        //Set desired features
		// VkPhysicalDeviceDescriptorHeapFeaturesEXT desired_descriptor_heap_features{
			// .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
			// .pNext = nullptr,
			// .descriptorHeap = VK_TRUE,
		// };
		// VkPhysicalDeviceMaintenance5FeaturesKHR desired_maintenance_5_features{
			// .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
			// .pNext = &desired_descriptor_heap_features,
			// .maintenance5 = VK_TRUE,
		// };
        VkPhysicalDeviceVulkan13Features desired_1_3_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = nullptr,//&desired_maintenance_5_features,
            .synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
        };
        VkPhysicalDeviceVulkan12Features desired_1_2_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &desired_1_3_features,
			.drawIndirectCount = VK_TRUE,
            .descriptorIndexing = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
			.scalarBlockLayout = VK_TRUE,
            .timelineSemaphore = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE,
        };
        VkPhysicalDeviceVulkan11Features desired_1_1_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &desired_1_2_features,
			.shaderDrawParameters = VK_TRUE,
        };
        VkPhysicalDeviceFeatures desired_base_features {
            .multiDrawIndirect = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        };
		VkPhysicalDeviceFeatures2 desired_features {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &desired_1_1_features,
			.features = desired_base_features,
		};

        //Set extensions
        //TODO improve with lookup table?
        std::vector<const char*> enabled_extensions{};
        for(std::size_t i = 0; i < associated_phys_device->extensions.size(); ++i)
            if(associated_phys_device->extensions[i] && (windowing || i != extension::swap_chain))
                enabled_extensions.push_back(extension::name[i].data());

        //Create logical device
        VkDeviceCreateInfo device_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &desired_features,
            .queueCreateInfoCount = queue_create_count,
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = VK_NULL_HANDLE,
        };

        logical_device ret{};
        __D2D_VULKAN_VERIFY(vkCreateDevice(*associated_phys_device, &device_create_info, nullptr, &ret));


        //Create queues
        for(std::size_t i = 0; i < queue_create_info_count; ++i)
            vkGetDeviceQueue(ret.handle, associated_phys_device->queue_family_infos[i].index, 0, &ret.queues[i]);


		//Load extended functions
		#define EXT_FN(name, suffix) \
		ret.vulkan_fns[sl::index_constant<extended_function::name>] = reinterpret_cast<PFN_##name##suffix>(vkGetDeviceProcAddr(ret.handle, #name #suffix)); \
		if(!ret.vulkan_fns[sl::index_constant<extended_function::name>]) \
			return errc::missing_vulkan_extension;

		__D2D_VK_EXT_FNS
		#undef EXT_FN

        return ret;
    }
}
