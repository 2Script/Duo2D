#include "Duo2D/vulkan/device/physical_device.hpp"

#include <cstring>
#include <streamline/functional/functor/construct_using.hpp>
#include <streamline/universal/make.hpp>

namespace d2d::vk {
    result<physical_device> physical_device::create(VkPhysicalDevice& device_handle, surface const& dummy_surface) noexcept {
        
        //Get device features and properties
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device_handle, &device_properties);
        vkGetPhysicalDeviceFeatures(device_handle, &device_features);


        //Get device queue family indicies
        auto device_queue_family_infos = sl::universal::make<sl::array<command_family::num_families, queue_family_info>>(
			sl::in_place_tag,
			false, 
			(static_cast<std::uint32_t>(sl::npos) >> 1)
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


        for(std::uint32_t idx = 0; idx < family_count; ++idx) {
            VkBool32 supports_present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device_handle, idx, dummy_surface, &supports_present);
			if(supports_present) 
				device_queue_family_infos[command_family::present] = queue_family_info{true, idx};

            for(std::size_t family_id = 0; family_id < command_family::num_distinct_families; ++family_id) {    
				if(!(families[idx].queueFlags & flag_bit[family_id])) continue;
				device_queue_family_infos[family_id] = queue_family_info{static_cast<bool>(supports_present), idx};
            }
        }

		//Check for queue family support
		//TODO: only check for queues that we actually need (based on graphics timeline)
		for(std::size_t i = 0; i < command_family::num_families; ++i)
			if(device_queue_family_infos[i].index == (static_cast<std::uint32_t>(sl::npos) >> 1))
				return static_cast<errc>(error::device_lacks_necessary_queue_base + i);
		
		}
		

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
            .name = device_properties.deviceName,
            .type = static_cast<device_type>(device_properties.deviceType),
            .limits = device_properties.limits,
            .queue_family_infos = device_queue_family_infos,
            .extensions = device_extensions,
            .features = std::bit_cast<features_t>(device_features),
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
