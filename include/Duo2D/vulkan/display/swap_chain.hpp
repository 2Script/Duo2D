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
        constexpr auto&& extent     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._extent); }
        constexpr auto&& image_count(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._image_count); }
        constexpr auto&& format     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._display_format); }
        constexpr auto&& mode       (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._present_mode); }
        constexpr auto&& image_views(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._image_views); }
        constexpr auto&& images     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._images); }

    private:
        extent2 _extent{};
        std::uint32_t _image_count = 0;
        vk::display_format _display_format;
        vk::present_mode _present_mode;
        std::vector<image_view> _image_views;
        std::vector<VkImage> _images;
    };
}