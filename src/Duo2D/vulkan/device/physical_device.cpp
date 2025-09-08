#include "Duo2D/vulkan/device/physical_device.hpp"

#include <cstring>

namespace d2d::vk {
    result<physical_device> physical_device::create(VkPhysicalDevice& device_handle, surface const& dummy_surface) noexcept {
        
        //Get device features and properties
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device_handle, &device_properties);
        vkGetPhysicalDeviceFeatures(device_handle, &device_features);


        //Get device queue family indicies
        queue_family_idxs_t device_idxs{};
        {
        std::uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device_handle, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device_handle, &family_count, families.data());

        for(std::size_t idx = 0; idx < families.size(); ++idx) {
            for(std::size_t family_id = 0; family_id < queue_family::num_families - 1; ++family_id) {
                if(families[idx].queueFlags & queue_family::flag_bit[family_id]) {
                    device_idxs[family_id] = idx;
                    goto next_family;
                }
            }

            {
            VkBool32 supports_present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device_handle, idx, dummy_surface, &supports_present);
            if(supports_present)
                device_idxs[queue_family::present] = idx;
            }

            next_family:;
        }
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
            .queue_family_idxs = device_idxs,
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
