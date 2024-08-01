#pragma once
#include <vulkan/vulkan.h>

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
        constexpr friend bool operator<(display_format lhs, display_format rhs) noexcept; 

        constexpr explicit operator VkSurfaceFormatKHR() const noexcept;
        constexpr explicit operator bool() const noexcept;
    };
}


namespace d2d::impl {
    using display_format_table_t = std::array<std::array<display_format, num_color_spaces>, num_pixel_formats>;
    constexpr display_format_table_t all_display_formats() noexcept;
}

#include "Duo2D/hardware/display/display_format.inl"