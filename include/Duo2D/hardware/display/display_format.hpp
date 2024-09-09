#pragma once
#include <vulkan/vulkan.h>
#include <tuple>

#include "Duo2D/hardware/display/pixel_format_ids.hpp"
#include "Duo2D/hardware/display/color_space_ids.hpp"
#include "Duo2D/hardware/display/pixel_format.hpp"
#include "Duo2D/hardware/display/color_space.hpp"


namespace d2d {
    struct display_format {
        VkFormat format_id;
        VkColorSpaceKHR color_space_id;

        pixel_format_info pixel_format;
        color_space_info color_space;


    public:
        constexpr friend bool operator<(display_format lhs, display_format rhs) noexcept {
            return std::tie(lhs.format_id, lhs.color_space_id) < std::tie(rhs.format_id, rhs.color_space_id);
        }

        constexpr explicit operator VkSurfaceFormatKHR() const noexcept { 
            return {format_id, color_space_id}; 
        }
        constexpr explicit operator bool() const noexcept { 
            return format_id != VK_FORMAT_UNDEFINED; 
        }
    };
}