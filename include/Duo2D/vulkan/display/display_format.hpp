#pragma once
#include <vulkan/vulkan.h>
#include <tuple>

#include "Duo2D/vulkan/display/pixel_format.hpp"
#include "Duo2D/vulkan/display/color_space.hpp"


namespace d2d::vk {
    struct display_format {
        pixel_format_info pixel_format;
        color_space_info color_space;


    public:
        constexpr friend bool operator<(display_format lhs, display_format rhs) noexcept {
            return std::tie(lhs.pixel_format.id, lhs.color_space.id) < std::tie(rhs.pixel_format.id, rhs.color_space.id);
        }

        constexpr explicit operator VkSurfaceFormatKHR() const noexcept { 
            return {pixel_format.id, color_space.id}; 
        }
        constexpr explicit operator bool() const noexcept { 
            return pixel_format.id != VK_FORMAT_UNDEFINED; 
        }
    };
}