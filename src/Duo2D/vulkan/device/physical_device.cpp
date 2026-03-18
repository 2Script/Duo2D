#include "Duo2D/vulkan/device/physical_device.hpp"

#include <cstring>
#include <streamline/functional/functor/construct_using.hpp>
#include <streamline/universal/make.hpp>

#include "Duo2D/core/window.hpp"


namespace d2d::vk {
    result<physical_device> physical_device::create(VkPhysicalDevice& device_handle, std::shared_ptr<vk::instance> instance, bool prefer_synchronous_rendering, bool window_capability) noexcept {
        
        //Get device features and properties
		//VkPhysicalDeviceDescriptorHeapPropertiesEXT device_descriptor_heap_properties{
		//	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT,
		//};
        VkPhysicalDeviceProperties2 device_properties{
	    	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			//.pNext = &device_descriptor_heap_properties,
		};
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties2(device_handle, &device_properties);
        vkGetPhysicalDeviceFeatures(device_handle, &device_features);


		constexpr static sl::uint32_t nidx = (static_cast<std::uint32_t>(sl::npos) >> 1);
        //Get device queue family indicies
        auto device_queue_family_infos = sl::universal::make<sl::array<command_family::num_families, queue_family_info>>(
			sl::in_place_tag,
			false, 
			nidx
		);
        
		{
        std::uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device_handle, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device_handle, &family_count, families.data());

        constexpr std::array<VkQueueFlagBits, command_family::num_distinct_families> flag_bit = {
            VK_QUEUE_GRAPHICS_BIT,
			VK_QUEUE_COMPUTE_BIT,
			VK_QUEUE_TRANSFER_BIT,
        }; 

		//Create dummy window
		window dummy_window{};
		if(window_capability)
			RESULT_TRY_MOVE(dummy_window, make<window>(instance, d2d::sz2u32{1280, 720}, "dummy"));

        for(std::uint32_t idx = 0; idx < family_count; ++idx) {
            VkBool32 supports_present = false;
			if(window_capability) {
            	vkGetPhysicalDeviceSurfaceSupportKHR(device_handle, idx, dummy_window.surface(), &supports_present);
				if(supports_present && (device_queue_family_infos[command_family::present].index == nidx || !prefer_synchronous_rendering)) 
					device_queue_family_infos[command_family::present] = queue_family_info{true, idx};
			}

            for(std::size_t family_id = 0; family_id < command_family::num_distinct_families; ++family_id) {    
				if(!(families[idx].queueFlags & flag_bit[family_id])) continue;
				if(device_queue_family_infos[family_id].index != nidx && prefer_synchronous_rendering) continue;
				device_queue_family_infos[family_id] = queue_family_info{static_cast<bool>(supports_present), idx};
            }
        }
		}

		//Check for queue family support
		//TODO: only check for queues that we actually need (based on graphics timeline)?
		const sl::size_t num_queue_families_to_verify = command_family::num_distinct_families + static_cast<sl::size_t>(window_capability);
		for(std::size_t i = 0; i < num_queue_families_to_verify; ++i)
			if(device_queue_family_infos[i].index == nidx)
				return static_cast<errc>(error::device_lacks_necessary_queue_base + i);
		
		

        //Get device extensions
        extensions_t device_extensions{};
        {
        std::uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device_handle, nullptr, &extension_count, nullptr);
    
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device_handle, nullptr, &extension_count, available_extensions.data());
    
        //TODO replace with lookup table
        for (std::size_t i = 0; i < extension::num_extensions; ++i){
            for (const auto& ext : available_extensions) {
                if(std::memcmp(ext.extensionName, extension::name[i].data(), extension::name[i].size()) == 0) {
                    device_extensions[i] = true;
                    goto next_extension;
                }
            }
            next_extension:;
        }
        }

        //Create device info
        physical_device ret{
            .name = device_properties.properties.deviceName,
            .type = static_cast<device_type>(device_properties.properties.deviceType),
            .extensions = device_extensions,
            .features = std::bit_cast<features_t>(device_features),
            .limits = device_properties.properties.limits,
			.descriptor_count_limits{{
				device_properties.properties.limits.maxDescriptorSetSamplers,

				device_properties.properties.limits.maxDescriptorSetSampledImages,
				device_properties.properties.limits.maxDescriptorSetStorageImages,
			}},
			.per_stage_descriptor_count_limits{{
				device_properties.properties.limits.maxPerStageDescriptorSamplers,
				
				device_properties.properties.limits.maxPerStageDescriptorSampledImages,
				device_properties.properties.limits.maxPerStageDescriptorStorageImages,
			}},

			.max_push_data_bytes = 0,//device_descriptor_heap_properties.maxPushDataSize,
			.asset_group_infos{{
				// {device_descriptor_heap_properties.samplerDescriptorSize, device_descriptor_heap_properties.samplerDescriptorAlignment},
				// {device_descriptor_heap_properties.imageDescriptorSize, device_descriptor_heap_properties.imageDescriptorAlignment},
				// {device_descriptor_heap_properties.bufferDescriptorSize, device_descriptor_heap_properties.bufferDescriptorAlignment},
			}},
			.descriptor_heap_infos{{
				// {
				// 	device_descriptor_heap_properties.minSamplerHeapReservedRange,
				// 	device_descriptor_heap_properties.samplerHeapAlignment,
				// 	device_descriptor_heap_properties.maxSamplerHeapSize
				// },
				// {
				// 	device_descriptor_heap_properties.minResourceHeapReservedRange,
				// 	device_descriptor_heap_properties.resourceHeapAlignment,
				// 	device_descriptor_heap_properties.maxResourceHeapSize
				// },
			}},
            .queue_family_infos = device_queue_family_infos,
        };
        ret.handle = device_handle;
        return ret;
    }
}



namespace d2d::vk {
    template<> typename device_query_traits<device_query::surface_capabilites>::return_type physical_device::query<device_query::surface_capabilites>(surface const& s) const noexcept {
        VkSurfaceCapabilitiesKHR device_surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, s, &device_surface_capabilities);
        return device_surface_capabilities;
    }

    template<> typename device_query_traits<device_query::display_formats>::return_type physical_device::query<device_query::display_formats>(surface const& s) const noexcept {
        std::set<display_format> formats;
        
        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(handle, s, &format_count, nullptr);

        std::vector<VkSurfaceFormatKHR> surface_formats;
        if (format_count != 0) {
            surface_formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(handle, s, &format_count, surface_formats.data());
        }

        //TODO replace with lookup table
        for(VkSurfaceFormatKHR f : surface_formats)
            formats.emplace(pixel_formats.find(f.format)->second, color_spaces.find(f.colorSpace)->second);

        //TODO replace with format to bool lookup table?
        return formats;
    }

    template<> typename device_query_traits<device_query::present_modes>::return_type physical_device::query<device_query::present_modes>(surface const& s) const noexcept {
        typename device_query_traits<device_query::present_modes>::return_type device_present_modes;
        
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(handle, s, &present_mode_count, nullptr);

        std::vector<VkPresentModeKHR> supported_present_modes;
        if (present_mode_count != 0) {
            supported_present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(handle, s, &present_mode_count, supported_present_modes.data());
        }

        for(VkPresentModeKHR p : supported_present_modes)
            if(p <= VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                device_present_modes[p] = true;
        
        return device_present_modes;
    }
}
