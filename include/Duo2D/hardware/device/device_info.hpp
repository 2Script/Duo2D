#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <array>
#include <set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/hardware/device/queue_family.hpp"
#include "Duo2D/hardware/device/extension.hpp"
#include "Duo2D/hardware/device/feature.hpp"
#include "Duo2D/hardware/device/device_type.hpp"
#include "Duo2D/hardware/display/display_format.hpp"
#include "Duo2D/hardware/display/present_mode.hpp"


namespace d2d {
    struct device_info {
        std::string_view name;
        device_type type = device_type::unknown;

        queue_family_idxs_t queue_family_idxs{};
        extensions_t extensions{};
        features_t features{};

        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::set<display_format> display_formats; //TODO replace with format to bool lookup table?
        std::array<bool, static_cast<std::size_t>(present_mode::num_present_modes)> present_modes;

        VkPhysicalDevice handle = VK_NULL_HANDLE;

    public:
        /*constexpr*/ inline friend bool operator<(device_info a, device_info b) noexcept {
            std::int32_t a_type_rating = a.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(a.type);
            std::int32_t b_type_rating = b.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(b.type);
            return std::tie(a_type_rating, a.handle) < std::tie(b_type_rating, b.handle);
        }

    };
}