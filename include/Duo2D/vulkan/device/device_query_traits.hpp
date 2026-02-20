#pragma once
#include <set>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/device_query.hpp"
#include "Duo2D/vulkan/display/display_format.hpp"
#include "Duo2D/vulkan/display/present_mode.hpp"


namespace d2d::vk {
    template<device_query Query>
    struct device_query_traits;
}

namespace d2d::vk {
    template<>
    struct device_query_traits<device_query::surface_capabilites> {
        using return_type = VkSurfaceCapabilitiesKHR;
    };

    template<>
    struct device_query_traits<device_query::display_formats> {
        using return_type = std::set<display_format>;
    };

    template<>
    struct device_query_traits<device_query::present_modes> {
        using return_type = std::array<bool, static_cast<std::size_t>(present_mode::num_present_modes)>;
    };
}
