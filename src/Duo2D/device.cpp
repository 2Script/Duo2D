#include "Duo2D/application.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/hardware/device/device_info.hpp"
#include "Duo2D/hardware/display/color_space.hpp"
#include "Duo2D/hardware/display/color_space_ids.hpp"
#include "Duo2D/hardware/display/display_format.hpp"
#include "Duo2D/hardware/display/pixel_format.hpp"
#include "Duo2D/hardware/display/pixel_format_ids.hpp"
#include "Duo2D/hardware/display/present_mode.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>
#include <algorithm>


namespace d2d {
    result<std::set<device_info>> application::devices() const noexcept {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::vector<VkPhysicalDevice> devices(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(vulkan_instance, &device_count, devices.data()));


        // Create dummy window
        window dummy{};
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        dummy.handle = glfwCreateWindow(1280, 720, "dummy", nullptr, nullptr);
        __D2D_GLFW_VERIFY(dummy.handle);
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(vulkan_instance, dummy.handle, nullptr, &dummy.surface));


        std::set<device_info> ret{};
        for(VkPhysicalDevice d : devices) {
            //Get device features and properties
            VkPhysicalDeviceProperties device_properties;
            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceProperties(d, &device_properties);
            vkGetPhysicalDeviceFeatures(d, &device_features);


            //Get device queue family indicies
            queue_family_idxs_t device_idxs{};
            {
            std::uint32_t family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(d, &family_count, nullptr);

            std::vector<VkQueueFamilyProperties> families(family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(d, &family_count, families.data());

            for(std::size_t idx = 0; idx < families.size(); ++idx) {
                for(std::size_t family_id = 0; family_id < queue_family::num_families - 1; ++family_id) {
                    if(families[idx].queueFlags & queue_family::flag_bit[family_id]) {
                        device_idxs[family_id] = idx;
                        goto next_family;
                    }
                }

                {
                VkBool32 supports_present = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(d, idx, dummy.surface, &supports_present);
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
            vkEnumerateDeviceExtensionProperties(d, nullptr, &extension_count, nullptr);
    
            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(d, nullptr, &extension_count, available_extensions.data());
    
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
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d, dummy.surface, &device_surface_capabilities);


            //Get device display formats (i.e. surface formats)
            std::set<display_format> device_formats;
            {
            std::uint32_t format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(d, dummy.surface, &format_count, nullptr);

            std::vector<VkSurfaceFormatKHR> formats;
            if (format_count != 0) {
                formats.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(d, dummy.surface, &format_count, formats.data());
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
            decltype(device_info::present_modes) device_present_modes;
            {
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(d, dummy.surface, &present_mode_count, nullptr);

            std::vector<VkPresentModeKHR> supported_present_modes;
            if (present_mode_count != 0) {
                supported_present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(d, dummy.surface, &present_mode_count, supported_present_modes.data());
            }

            for(VkPresentModeKHR p : supported_present_modes)
                if(p <= VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                    device_present_modes[p] = true;
            }

            //Create device info
            ret.emplace(
                device_properties.deviceName, 
                static_cast<device_type>(device_properties.deviceType),

                device_idxs,
                device_extensions,
                std::bit_cast<d2d::features_t>(device_features),

                device_surface_capabilities,
                device_formats,
                device_present_modes,

                d
            );
        }

        //Destroy dummy window
        vkDestroySurfaceKHR(vulkan_instance, dummy.surface, nullptr);
        glfwDestroyWindow(dummy.handle);

        return ret;
    }
}


namespace d2d {
    result<void> application::initialize_device() noexcept {
        if(!physical_device.handle)
            return error::device_not_selected;
        
        //Check for queue family support
        for(std::size_t i = 0; i < queue_family::num_families; ++i)
            if(!physical_device.queue_family_idxs[i].has_value())
                return static_cast<errc>(error::device_lacks_necessary_queue_base + i);

        //Create desired queues for each queue family
        constexpr static float priority = 1.f;
        std::array<VkDeviceQueueCreateInfo, queue_family::num_families> queue_create_infos{};
        for(std::size_t i = 0; i < queue_family::num_families; ++i) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = *(physical_device.queue_family_idxs[i]);
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &priority;
            queue_create_infos[i] = queue_create_info;
        }

        //Set desired features
        VkPhysicalDeviceFeatures desired_features{};

        //Set extensions
        //TODO improve with lookup table?
        std::vector<const char*> enabled_extensions;
        for(std::size_t i = 0; i < physical_device.extensions.size(); ++i)
            if(physical_device.extensions[i])
                enabled_extensions.push_back(extension::name[i].data());

        //Create logical device
        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = queue_create_infos.size();
        device_create_info.pEnabledFeatures = &desired_features;
        device_create_info.enabledExtensionCount = enabled_extensions.size();
        device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
        device_create_info.enabledLayerCount = 0;

        __D2D_VULKAN_VERIFY(vkCreateDevice(physical_device.handle, &device_create_info, nullptr, &logical_device));

        //Create queues
        for(std::size_t i = 0; i < queue_family::num_families; ++i)
            vkGetDeviceQueue(logical_device, *(physical_device.queue_family_idxs[i]), 0, &queues[i]);

        
        //Check display format support (TEMP: set to default [VK_FORMAT_B8G8R8A8_SRGB & VK_COLOR_SPACE_SRGB_NONLINEAR_KHR])
        {
        constexpr static display_format defualt_display_format = {impl::pixel_format_ids[50], impl::color_space_ids[0], pixel_formats[50], color_spaces[0]};
        const auto it = physical_device.display_formats.find(defualt_display_format);
        if(it == physical_device.display_formats.end())
            return error::device_lacks_display_format;
        device_format = *it;
        }

        //Check present mode support (TEMP: set to default)
        if(physical_device.present_modes[static_cast<std::size_t>(present_mode::mailbox)])
            device_present_mode = present_mode::mailbox;
        else if(physical_device.present_modes[static_cast<std::size_t>(present_mode::fifo)])
            device_present_mode = present_mode::fifo;
        else return error::device_lacks_present_mode;

        return result<void>{std::in_place_type<void>};
    }
}