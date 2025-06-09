#pragma once
#include <string_view>
#include <array>
#include <set>
#include <compare>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/error.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/device/queue_family.hpp"
#include "Duo2D/vulkan/device/extension.hpp"
#include "Duo2D/vulkan/device/feature.hpp"
#include "Duo2D/vulkan/device/device_type.hpp"
#include "Duo2D/vulkan/display/display_format.hpp"
#include "Duo2D/vulkan/display/present_mode.hpp"


namespace d2d {
    struct window;

    struct physical_device : vulkan_ptr_base<VkPhysicalDevice> {
        static result<physical_device> create(VkPhysicalDevice& device_handle, window& dummy_window) noexcept;
        
    public:
        std::string_view name;
        device_type type = device_type::unknown;

        VkPhysicalDeviceLimits limits{};
        queue_family_idxs_t queue_family_idxs{};
        extensions_t extensions{};
        features_t features{};

        VkSurfaceCapabilitiesKHR surface_capabilities; //TODO replace with user-defined type
        std::set<display_format> display_formats; //TODO replace with format to bool lookup table?
        std::array<bool, static_cast<std::size_t>(present_mode::num_present_modes)> present_modes;

    public:
        friend std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept;
    };
}
