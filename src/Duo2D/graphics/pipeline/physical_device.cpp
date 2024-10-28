#include "Duo2D/graphics/pipeline/physical_device.hpp"

#include <cstring>
#include "Duo2D/graphics/pipeline/window.hpp"

namespace d2d {
    result<physical_device> physical_device::create(VkPhysicalDevice& device_handle, window& dummy_window) noexcept {
        
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device_handle, idx, dummy_window.window_surface, &supports_present);
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


        //Get device surface capabilities
        VkSurfaceCapabilitiesKHR device_surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_handle, dummy_window.window_surface, &device_surface_capabilities);


        //Get device display formats (i.e. surface formats)
        std::set<display_format> device_formats;
        {
        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_handle, dummy_window.window_surface, &format_count, nullptr);

        std::vector<VkSurfaceFormatKHR> formats;
        if (format_count != 0) {
            formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device_handle, dummy_window.window_surface, &format_count, formats.data());
        }

        //TODO replace with lookup table
        for(VkSurfaceFormatKHR f : formats) {
            for(std::size_t i = 0; i < impl::num_pixel_formats; ++i) {
                for(std::size_t j = 0; j < impl::num_color_spaces; ++j) {
                    if(impl::pixel_format_ids[i] == f.format && impl::color_space_ids[j] == f.colorSpace) {
                        device_formats.emplace(impl::pixel_format_ids[i], impl::color_space_ids[j], pixel_formats[i], color_spaces[j]);
                        goto next_format;
                    }
                }
            }
            next_format:;
        }
        }


        //Get device present modes
        decltype(physical_device::present_modes) device_present_modes;
        {
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_handle, dummy_window.window_surface, &present_mode_count, nullptr);

        std::vector<VkPresentModeKHR> supported_present_modes;
        if (present_mode_count != 0) {
            supported_present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device_handle, dummy_window.window_surface, &present_mode_count, supported_present_modes.data());
        }

        for(VkPresentModeKHR p : supported_present_modes)
            if(p <= VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                device_present_modes[p] = true;
        }

        //Create device info
        physical_device ret{};
        ret.name = device_properties.deviceName;
        ret.type = static_cast<device_type>(device_properties.deviceType);
        ret.queue_family_idxs = device_idxs;
        ret.extensions = device_extensions;
        ret.features = std::bit_cast<d2d::features_t>(device_features);
        ret.surface_capabilities = device_surface_capabilities;
        ret.display_formats = device_formats;
        ret.present_modes = device_present_modes;
        ret.handle = device_handle;
        return ret;
    }
}

namespace d2d {
    std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept {
        std::int32_t a_type_rating = a.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(a.type);
        std::int32_t b_type_rating = b.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(b.type);
        return a_type_rating <=> b_type_rating;
    }
}