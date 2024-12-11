#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/framebuffer.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/surface.hpp"
#include "Duo2D/graphics/pipeline/image_view.hpp"
#include "Duo2D/arith/size.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkSwapchainKHR);


namespace d2d {
    struct swap_chain : pipeline_obj<VkSwapchainKHR, vkDestroySwapchainKHR> {
        static result<swap_chain> create(logical_device& logi_deivce, physical_device& phys_device, render_pass& window_render_pass, surface& window_surface, window& w) noexcept;

    private:
        extent2 extent;
        //TEMP:
        std::vector<VkImage> images;
        std::vector<image_view> image_views;
        std::vector<framebuffer> framebuffers;
    
    private:
        friend struct command_buffer;
        friend window; //TODO just make an extent() function
    };
}