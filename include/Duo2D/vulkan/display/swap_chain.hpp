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
	public:
		constexpr static std::array<vk::pixel_format_info, 2> default_pixel_format_priority = {vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_SRGB)->second, vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_UNORM)->second};
		constexpr static vk::color_space_info default_color_space = vk::color_spaces.find(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)->second;
		constexpr static std::array<vk::present_mode, static_cast<std::size_t>(vk::present_mode::num_present_modes)> default_present_mode_priority{vk::present_mode::mailbox, vk::present_mode::fifo, vk::present_mode::fifo_relaxed, vk::present_mode::immediate}; 
	

	public:
		template<typename... Args>
        static result<swap_chain> create(Args&&... args) noexcept {
			swap_chain ret{};
			RESULT_VERIFY(ret.reset(sl::forward<Args>(args)...));
			return ret;
		}
	public:
		result<void> reset(
			std::shared_ptr<logical_device> logi_deivce,
			std::weak_ptr<physical_device> phys_device,
			surface& window_surface,
			GLFWwindow& window_instance,
			std::span<const pixel_format_info> pixel_format_priority = default_pixel_format_priority,
			color_space_info color_space = default_color_space,
			std::span<const present_mode> present_mode_priority = default_present_mode_priority
		) noexcept;

    
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