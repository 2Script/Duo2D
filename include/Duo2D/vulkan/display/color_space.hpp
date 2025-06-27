#pragma once
#include <array>
#include <cstdint>
#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/display/color_space_ids.hpp"

namespace d2d::vk {
    struct color_space_info {
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
    constexpr std::array<color_space_info, impl::num_color_spaces> color_spaces = {{
       {color_space_info::srgb, color_space_info::nonlinear},          //VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
       {color_space_info::display_p3, color_space_info::nonlinear},    //VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT
       {color_space_info::scrgb, color_space_info::linear},            //VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT
       {color_space_info::display_p3, color_space_info::linear},       //VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT
       {color_space_info::dci_p3, color_space_info::nonlinear},        //VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT
       {color_space_info::bt709, color_space_info::linear},            //VK_COLOR_SPACE_BT709_LINEAR_EXT
       {color_space_info::bt709, color_space_info::nonlinear},         //VK_COLOR_SPACE_BT709_NONLINEAR_EXT
       {color_space_info::bt2020, color_space_info::linear},           //VK_COLOR_SPACE_BT2020_LINEAR_EXT
       {color_space_info::bt2020, color_space_info::st2084_pq, color_space_info::hdr10},        //VK_COLOR_SPACE_HDR10_ST2084_EXT
       {color_space_info::bt2020, color_space_info::st2084_pq, color_space_info::dolby_vision}, //VK_COLOR_SPACE_DOLBYVISION_EXT
       {color_space_info::bt2020, color_space_info::hlg, color_space_info::hdr10},              //VK_COLOR_SPACE_HDR10_HLG_EXT
       {color_space_info::adobe_rgb, color_space_info::linear},        //VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT
       {color_space_info::adobe_rgb, color_space_info::nonlinear},     //VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT
       {color_space_info::passthrough, color_space_info::unspecified}, //VK_COLOR_SPACE_PASS_THROUGH_EXT
       {color_space_info::scrgb, color_space_info::nonlinear},         //VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT
       {color_space_info::native, color_space_info::unspecified},      //VK_COLOR_SPACE_DISPLAY_NATIVE_AMD
    }};
}