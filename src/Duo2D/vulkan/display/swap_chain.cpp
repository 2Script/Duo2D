#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"

namespace d2d::vk {
    result<swap_chain> swap_chain::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, render_pass& window_render_pass, surface& window_surface, GLFWwindow* w) noexcept {
        swap_chain ret{};
        ret.dependent_handle = logi_device;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device_ptr, phys_device);
        
        VkSurfaceCapabilitiesKHR device_capabilities = phys_device_ptr->surface_capabilities;

        //Create swap extent
        {
        if(device_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>().max())
            ret._extent = {device_capabilities.currentExtent.width, device_capabilities.currentExtent.height};
        else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(w, &width, &height);
            __D2D_GLFW_VERIFY((width != 0 && height != 0));
            ret._extent = {
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
            .imageFormat = logi_device->format.format_id,
            .imageColorSpace = logi_device->format.color_space_id,
            .imageExtent = static_cast<VkExtent2D>(ret._extent),
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = device_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = static_cast<VkPresentModeKHR>(logi_device->mode),
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        const std::array<std::uint32_t, queue_family::present + 1> core_queue_family_idxs = {
            *(phys_device_ptr->queue_family_idxs[queue_family::graphics]), *(phys_device_ptr->queue_family_idxs[queue_family::present])
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

        __D2D_VULKAN_VERIFY(vkCreateSwapchainKHR(*logi_device, &swap_chain_create_info, nullptr, &ret));
        }

        //Get swap chain images
        {
        ret._image_count = 0;
        vkGetSwapchainImagesKHR(*logi_device, ret.handle, &ret._image_count, nullptr);
        ret.images.resize(ret._image_count);
        vkGetSwapchainImagesKHR(*logi_device, ret.handle, &ret._image_count, ret.images.data());
        }

        //Create swap chain image views
        ret.image_views.resize(ret._image_count);
        for (size_t i = 0; i < ret._image_count; i++) {
            RESULT_TRY_MOVE(ret.image_views[i], make<image_view>(logi_device, ret.images[i], logi_device->format.format_id));
        }

        //Create framebuffers
        ret.framebuffers.resize(ret._image_count);
        for (size_t i = 0; i < ret._image_count; i++) {
            RESULT_TRY_MOVE(ret.framebuffers[i], make<framebuffer>(logi_device, ret.image_views[i], window_render_pass, ret._extent));
        }

        return ret;
    }
}