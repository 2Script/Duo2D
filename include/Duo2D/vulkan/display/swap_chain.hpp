#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "Duo2D/vulkan/display/color_space.hpp"
#include "Duo2D/vulkan/display/depth_image.hpp"
#include "Duo2D/vulkan/display/display_format.hpp"
#include "Duo2D/vulkan/display/framebuffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"
#include "Duo2D/vulkan/display/present_mode.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/image_view.hpp"
#include "Duo2D/arith/size.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE(VkSwapchainKHR);


namespace d2d::vk {
    struct swap_chain : vulkan_ptr<VkSwapchainKHR, vkDestroySwapchainKHR> {
        static result<swap_chain> create(std::shared_ptr<logical_device> logi_deivce, std::weak_ptr<physical_device> phys_device, std::span<const pixel_format_info> pixel_format_priority, color_space_info color_space, std::span<const present_mode> present_mode_priority, surface& window_surface, GLFWwindow* w) noexcept;

    public:
        constexpr extent2 const& extent() const noexcept { return _extent; }
        constexpr extent2      & extent()       noexcept { return _extent; }

        constexpr vk::display_format const& format() const noexcept { return _display_format; }
        constexpr vk::display_format      & format()       noexcept { return _display_format; }

        constexpr vk::present_mode const& mode() const noexcept { return _present_mode; }
        constexpr vk::present_mode      & mode()       noexcept { return _present_mode; }

        constexpr std::uint32_t const& image_count() const noexcept { return _image_count; }
        constexpr std::uint32_t      & image_count()       noexcept { return _image_count; }

        constexpr std::vector<image_view> const& image_views() const noexcept { return _image_views; }
        constexpr std::vector<image_view>      & image_views()       noexcept { return _image_views; }

    private:
        extent2 _extent{};
        std::uint32_t _image_count = 0;
        vk::display_format _display_format;
        vk::present_mode _present_mode;
        std::vector<image_view> _image_views;
        std::vector<VkImage> images;
    };
}