#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>
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
        static result<swap_chain> create(std::shared_ptr<logical_device> logi_deivce, std::weak_ptr<physical_device> phys_device, render_pass& window_render_pass, surface& window_surface, GLFWwindow* w) noexcept;

    public:
        constexpr extent2 const& extent() const noexcept { return _extent; }
        constexpr extent2      & extent()       noexcept { return _extent; }

        constexpr std::uint32_t const& image_count() const noexcept { return _image_count; }
        constexpr std::uint32_t      & image_count()       noexcept { return _image_count; }

    private:
        extent2 _extent{};
        std::uint32_t _image_count = 0;
        //TEMP:
        std::vector<VkImage> images;
        std::vector<image_view> image_views;
        std::vector<framebuffer> framebuffers;
    
    private:
        friend struct command_buffer;
    };
}