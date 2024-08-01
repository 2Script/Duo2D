#pragma once
#include "Duo2D/hardware/display/display_format.hpp"
#include <tuple>
#include <vulkan/vulkan_core.h>


namespace d2d::impl {
    constexpr display_format_table_t all_display_formats() noexcept {
        display_format_table_t ret;
        for(std::size_t i = 0; i < num_pixel_formats; ++i)
            for(std::size_t j = 0; j < num_color_spaces; ++j)
                ret[i][j] = {pixel_format_ids[i], color_space_ids[j], pixel_formats[i], color_spaces[j]};
        return ret;
    }
}

namespace d2d {
    constexpr bool operator<(display_format lhs, display_format rhs) noexcept {
        return std::tie(lhs.format_id, lhs.color_space_id) < std::tie(rhs.format_id, rhs.color_space_id);
    }

    
    constexpr display_format::operator VkSurfaceFormatKHR() const noexcept {
        return {format_id, color_space_id};
    }

    
    constexpr display_format::operator bool() const noexcept {
        return format_id != VK_FORMAT_UNDEFINED;
    }
}