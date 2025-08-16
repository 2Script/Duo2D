#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d::vk {
    result<logical_device> logical_device::create(std::weak_ptr<physical_device> associated_phys_device) noexcept {
        auto phys_device_ptr = associated_phys_device.lock();
        if(!phys_device_ptr)
            return error::device_not_selected;
        
        //Check for queue family support
        for(std::size_t i = 0; i < queue_family::num_families; ++i)
            if(!phys_device_ptr->queue_family_idxs[i].has_value())
                return static_cast<errc>(error::device_lacks_necessary_queue_base + i);

        //Create desired queues for each queue family
        constexpr static float priority = 1.f;
        std::array<VkDeviceQueueCreateInfo, queue_family::num_families> queue_create_infos{};
        for(std::size_t i = 0; i < queue_family::num_families; ++i) {
            VkDeviceQueueCreateInfo queue_create_info{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = *(phys_device_ptr->queue_family_idxs[i]),
                .queueCount = 1,
                .pQueuePriorities = &priority,
            };
            queue_create_infos[i] = queue_create_info;
        }

        //Set desired features
        VkPhysicalDeviceVulkan13Features desired_1_3_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = VK_TRUE,
        };
        VkPhysicalDeviceVulkan12Features desired_1_2_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &desired_1_3_features,
            .descriptorIndexing = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
            .timelineSemaphore = VK_TRUE,
        };
        VkPhysicalDeviceFeatures desired_base_features {
            .samplerAnisotropy = VK_TRUE,
        };

        //Set extensions
        //TODO improve with lookup table?
        std::vector<const char*> enabled_extensions;
        for(std::size_t i = 0; i < phys_device_ptr->extensions.size(); ++i)
            if(phys_device_ptr->extensions[i])
                enabled_extensions.push_back(extension::name[i].data());

        //Create logical device
        VkDeviceCreateInfo device_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &desired_1_2_features,
            .queueCreateInfoCount = queue_create_infos.size(),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = &desired_base_features,
        };

        logical_device ret{};
        __D2D_VULKAN_VERIFY(vkCreateDevice(*phys_device_ptr, &device_create_info, nullptr, &ret));


        
        //Check display format support (TEMP: set to default [VK_FORMAT_B8G8R8A8_SRGB & VK_COLOR_SPACE_SRGB_NONLINEAR_KHR])
        {
        constexpr static display_format defualt_display_format = {impl::pixel_format_ids[50], impl::color_space_ids[0], pixel_formats[50], color_spaces[0]};
        const auto it = phys_device_ptr->display_formats.find(defualt_display_format);
        if(it == phys_device_ptr->display_formats.end())
            return error::device_lacks_display_format;
        ret.format = *it;
        }

        //Check present mode support
        if(phys_device_ptr->present_modes[static_cast<std::size_t>(present_mode::mailbox)])
            ret.mode = present_mode::mailbox;
        else if(phys_device_ptr->present_modes[static_cast<std::size_t>(present_mode::fifo)])
            ret.mode = present_mode::fifo;
        else return error::device_lacks_present_mode;

        //Create queues
        for(std::size_t i = 0; i < queue_family::num_families; ++i)
            vkGetDeviceQueue(ret.handle, *(phys_device_ptr->queue_family_idxs[i]), 0, &ret.queues[i]);


        return ret;
    }
}
