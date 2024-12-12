#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/core/window.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    result<swap_chain> swap_chain::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass, surface& window_surface, window& w) noexcept {
        swap_chain ret{};
        ret.dependent_handle = logi_device;
        
        VkSurfaceCapabilitiesKHR device_capabilities = phys_device.surface_capabilities;

        //Create swap extent
        {
        if(device_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>().max())
            ret.extent = {device_capabilities.currentExtent.width, device_capabilities.currentExtent.height};
        else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(w, &width, &height);
            ret.extent = {
                std::clamp(static_cast<uint32_t>(width), device_capabilities.minImageExtent.width, device_capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(height), device_capabilities.minImageExtent.height, device_capabilities.maxImageExtent.height)
            };
        }
        }

        //Create swap chain
        {
        std::uint32_t image_count = device_capabilities.minImageCount + 1;
        if(device_capabilities.maxImageCount > 0)
            image_count = std::min(device_capabilities.maxImageCount, image_count);

        VkSwapchainCreateInfoKHR swap_chain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = window_surface,
            .minImageCount = image_count,
            .imageFormat = logi_device.format.format_id,
            .imageColorSpace = logi_device.format.color_space_id,
            .imageExtent = static_cast<VkExtent2D>(ret.extent),
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = device_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = static_cast<VkPresentModeKHR>(logi_device.mode),
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        const std::array<std::uint32_t, queue_family::present + 1> core_queue_family_idxs = {
            *(phys_device.queue_family_idxs[queue_family::graphics]), *(phys_device.queue_family_idxs[queue_family::present])
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

        __D2D_VULKAN_VERIFY(vkCreateSwapchainKHR(logi_device, &swap_chain_create_info, nullptr, &ret));
        }

        //Get swap chain images
        {
        std::uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(logi_device, ret.handle, &image_count, nullptr);
        ret.images.resize(image_count);
        vkGetSwapchainImagesKHR(logi_device, ret.handle, &image_count, ret.images.data());
        }

        //Create swap chain image views
        ret.image_views.resize(ret.images.size());
        for (size_t i = 0; i < ret.images.size(); i++) {
            __D2D_TRY_MAKE(ret.image_views[i], make<image_view>(logi_device, ret.images[i], logi_device.format.format_id), iv);
        }

        //Create framebuffers
        ret.framebuffers.resize(ret.image_views.size());
        for (size_t i = 0; i < ret.image_views.size(); i++) {
            __D2D_TRY_MAKE(ret.framebuffers[i], make<framebuffer>(logi_device, ret.image_views[i], window_render_pass, ret.extent), f);
        }

        return ret;
    }
}