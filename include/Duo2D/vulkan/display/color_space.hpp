#pragma once
#include <cstdint>

#include <frozen/unordered_map.h>
#include <vulkan/vulkan.h>


namespace d2d::vk::impl{
    constexpr std::size_t num_color_spaces = ((VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT & 0xF) + 1) + 1;
}

namespace d2d::vk {
    struct color_space_info {
        VkColorSpaceKHR id;

        enum : std::uint_fast8_t {
            srgb, scrgb, adobe_rgb, display_p3, dci_p3, bt709, bt2020, passthrough, native
        } specification;
        enum : std::uint_fast8_t {
            unspecified, linear, nonlinear, st2084_pq, hlg
        } encoding;
        enum : std::uint_fast8_t {
            none, hdr10, dolby_vision
        } hdr = none;
    };
}

namespace d2d::vk {
    constexpr frozen::unordered_map<VkColorSpaceKHR, color_space_info, impl::num_color_spaces> color_spaces = {{
       {VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,       {VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,       color_space_info::srgb, color_space_info::nonlinear}      },
       {VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT, {VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT, color_space_info::display_p3, color_space_info::nonlinear}},
       {VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT, {VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT, color_space_info::scrgb, color_space_info::linear}        },
       {VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,    {VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,    color_space_info::display_p3, color_space_info::linear}   },
       {VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,     {VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,     color_space_info::dci_p3, color_space_info::nonlinear}    },

       {VK_COLOR_SPACE_BT709_LINEAR_EXT,    {VK_COLOR_SPACE_BT709_LINEAR_EXT,    color_space_info::bt709, color_space_info::linear}   },
       {VK_COLOR_SPACE_BT709_NONLINEAR_EXT, {VK_COLOR_SPACE_BT709_NONLINEAR_EXT, color_space_info::bt709, color_space_info::nonlinear}},
       {VK_COLOR_SPACE_BT2020_LINEAR_EXT,   {VK_COLOR_SPACE_BT2020_LINEAR_EXT,   color_space_info::bt2020, color_space_info::linear}  },

       {VK_COLOR_SPACE_HDR10_ST2084_EXT, {VK_COLOR_SPACE_HDR10_ST2084_EXT, color_space_info::bt2020, color_space_info::st2084_pq, color_space_info::hdr10}       },
       {VK_COLOR_SPACE_DOLBYVISION_EXT,  {VK_COLOR_SPACE_DOLBYVISION_EXT,  color_space_info::bt2020, color_space_info::st2084_pq, color_space_info::dolby_vision}},
       {VK_COLOR_SPACE_HDR10_HLG_EXT,    {VK_COLOR_SPACE_HDR10_HLG_EXT,    color_space_info::bt2020, color_space_info::hlg, color_space_info::hdr10}             },

       {VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,    {VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,    color_space_info::adobe_rgb, color_space_info::linear}       },
       {VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT, {VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT, color_space_info::adobe_rgb, color_space_info::nonlinear}    },
       {VK_COLOR_SPACE_PASS_THROUGH_EXT,       {VK_COLOR_SPACE_PASS_THROUGH_EXT,       color_space_info::passthrough, color_space_info::unspecified}},

       {VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT, {VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT, color_space_info::scrgb, color_space_info::nonlinear}   },
       {VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,          {VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,          color_space_info::native, color_space_info::unspecified}},
    }};
}