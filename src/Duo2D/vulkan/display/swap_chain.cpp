#include "Duo2D/vulkan/display/swap_chain.hpp"

#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/device/device_query.hpp"
#include "Duo2D/vulkan/display/color_space.hpp"
#include "Duo2D/vulkan/display/display_format.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"
#include "Duo2D/vulkan/display/present_mode.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d::vk {
    result<swap_chain> swap_chain::create(std::shared_ptr<logical_device> logi_device, std::weak_ptr<physical_device> phys_device, std::span<const pixel_format_info> pixel_format_priority, color_space_info color_space, std::span<const present_mode> present_mode_priority, surface& window_surface, GLFWwindow* w) noexcept {
        swap_chain ret{};
        ret.dependent_handle = logi_device;
        __D2D_WEAK_PTR_TRY_LOCK(phys_device_ptr, phys_device);
        
        VkSurfaceCapabilitiesKHR device_capabilities = phys_device_ptr->query<device_query::surface_capabilites>(window_surface);

        //Create swap extent
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



        //Check display format support (TEMP: set to default [VK_FORMAT_B8G8R8A8_SRGB & VK_COLOR_SPACE_SRGB_NONLINEAR_KHR])
        auto display_formats = phys_device_ptr->query<device_query::display_formats>(window_surface);
        //Vulkan spec states number of supported formats must be greater than or equal to 1
        if(display_formats.size() == 0) 
            return error::device_lacks_display_format;
        //Vulkan spec states supported formats must not contain a format with VK_FORMAT_UNDEFINED
        for(display_format d : display_formats)
            if(d.pixel_format.id == VK_FORMAT_UNDEFINED) [[unlikely]]
                return error::device_lacks_display_format;
        for(std::size_t i = 0; i < pixel_format_priority.size(); ++i) {
            const auto it = display_formats.find({pixel_format_priority[i], color_space});
            if(it != display_formats.end()) {
                ret._display_format = *it;
                goto found_display_format;
            }
        }
        ret._display_format = *display_formats.begin();
        //return error::device_lacks_display_format;
    found_display_format:

        //Check present mode support
        auto present_modes = phys_device_ptr->query<device_query::present_modes>(window_surface);
        //Vulkan spec garauntees that fifo present mode is supported
        if(!present_modes[static_cast<std::size_t>(vk::present_mode::fifo)])
            return error::device_lacks_present_mode;
        for(present_mode p : present_mode_priority) {
            if(present_modes[static_cast<std::size_t>(p)]) {
                ret._present_mode = p;
                goto found_present_mode;
            }
        }
        ret._present_mode = vk::present_mode::fifo;
    found_present_mode:


        //Create swap chain
        {
        std::uint32_t image_count = device_capabilities.minImageCount + 1;
        if(device_capabilities.maxImageCount > 0)
            image_count = std::min(device_capabilities.maxImageCount, image_count);

        VkSwapchainCreateInfoKHR swap_chain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = window_surface,
            .minImageCount = image_count,
            .imageFormat = ret._display_format.pixel_format.id,
            .imageColorSpace = ret._display_format.color_space.id,
            .imageExtent = static_cast<VkExtent2D>(ret._extent),
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
            .preTransform = device_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = static_cast<VkPresentModeKHR>(ret._present_mode),
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        __D2D_VULKAN_VERIFY(vkCreateSwapchainKHR(*logi_device, &swap_chain_create_info, nullptr, &ret));
        }

        //Get swap chain images
        {
        ret._image_count = 0;
        vkGetSwapchainImagesKHR(*logi_device, ret.handle, &ret._image_count, nullptr);
        ret._images.resize(ret._image_count);
        vkGetSwapchainImagesKHR(*logi_device, ret.handle, &ret._image_count, ret._images.data());
        }

        //Create swap chain image views
        ret._image_views.resize(ret._image_count);
        for (size_t i = 0; i < ret._image_count; i++) {
            RESULT_TRY_MOVE(ret._image_views[i], make<image_view>(logi_device, ret._images[i], ret._display_format.pixel_format.id));
        }

        return ret;
    }
}