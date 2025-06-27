#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/display/framebuffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/arith/size.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkSwapchainKHR);


namespace d2d::vk {
    struct swap_chain : vulkan_ptr<VkSwapchainKHR, vkDestroySwapchainKHR> {
        static result<swap_chain> create(logical_device& logi_deivce, physical_device& phys_device, render_pass& window_render_pass, surface& window_surface, ::d2d::window& w) noexcept;

    private:
        extent2 extent{};
        std::uint32_t image_count = 0;
        //TEMP:
        std::vector<VkImage> images;
        std::vector<image_view> image_views;
        std::vector<framebuffer> framebuffers;
    
    private:
        friend struct command_buffer;
        friend ::d2d::window; //TODO just make an extent() function?
    };
}