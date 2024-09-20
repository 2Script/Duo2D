#include "Duo2D/application.hpp"

#include "Duo2D/error.hpp"
#include "Duo2D/hardware/device/queue_family.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <vulkan/vulkan_core.h>


namespace d2d {
    result<void> application::add_window(std::string_view title) noexcept {
        if(!logical_device)
            return error::device_not_initialized;

        window w{};

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        w.handle = glfwCreateWindow(1280, 720, title.data(), nullptr, nullptr);

        __D2D_GLFW_VERIFY(w.handle);

        //Create surface
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(vulkan_instance, w.handle, nullptr, &w.surface));

        //Create swap extent
        const VkSurfaceCapabilitiesKHR& capabilities = physical_device.surface_capabilities;
        
        if(capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>().max())
            w.swap_extent = capabilities.currentExtent;
        else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(w.handle, &width, &height);
            w.swap_extent = {
                std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }

        //Create swap chain
        {
        std::uint32_t image_count = capabilities.minImageCount + 1;
        if(capabilities.maxImageCount > 0)
            image_count = std::min(capabilities.maxImageCount, image_count);

        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = w.surface;
        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = device_format.format_id;
        swap_chain_create_info.imageColorSpace = device_format.color_space_id;
        swap_chain_create_info.imageExtent = w.swap_extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_chain_create_info.preTransform = capabilities.currentTransform;
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = static_cast<VkPresentModeKHR>(device_present_mode);
        swap_chain_create_info.clipped = VK_TRUE;
        swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

        const std::array<std::uint32_t, queue_family::present + 1> core_queue_family_idxs = {
            *(physical_device.queue_family_idxs[queue_family::graphics]), *(physical_device.queue_family_idxs[queue_family::present])
        };
        if(core_queue_family_idxs[queue_family::graphics] != core_queue_family_idxs[queue_family::present]){
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = core_queue_family_idxs.size();
            swap_chain_create_info.pQueueFamilyIndices = core_queue_family_idxs.data();
        } else {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swap_chain_create_info.queueFamilyIndexCount = 0;
            swap_chain_create_info.pQueueFamilyIndices = nullptr;
        }

        __D2D_VULKAN_VERIFY(vkCreateSwapchainKHR(logical_device, &swap_chain_create_info, nullptr, &w.swap_chain));
        }

        //Get swap chain images
        {
        std::uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(logical_device, w.swap_chain, &image_count, nullptr);
        w.images.resize(image_count);
        vkGetSwapchainImagesKHR(logical_device, w.swap_chain, &image_count, w.images.data());
        }

        //Create swap chain image views
        w.image_views.resize(w.images.size());
        for (size_t i = 0; i < w.images.size(); i++) {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = w.images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = device_format.format_id;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            __D2D_VULKAN_VERIFY(vkCreateImageView(logical_device, &image_view_create_info, nullptr, &w.image_views[i]));
        }


        //Add window
        if(!windows.emplace(title, w).second) 
            return error::window_already_exists;

        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    result<void> application::remove_window(std::string_view title) noexcept {
        if (auto it = windows.find(std::string(title)); it != windows.end()) {
            vkDestroySurfaceKHR(vulkan_instance, it->second.surface, nullptr);
            glfwDestroyWindow(it->second.handle);
            windows.erase(it);
            return result<void>{std::in_place_type<void>};
        }
 
        return error::window_not_found;
    }
}

namespace d2d {
    result<void> application::add_window() noexcept {
        return add_window(name);
    }

    result<void> application::remove_window() noexcept {
        return remove_window(name);
    }
}