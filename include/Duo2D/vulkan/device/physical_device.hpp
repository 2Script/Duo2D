#pragma once
#include <string_view>
#include <array>
#include <set>
#include <compare>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/device/device_query.hpp"
#include "Duo2D/vulkan/device/queue_family.hpp"
#include "Duo2D/vulkan/device/extension.hpp"
#include "Duo2D/vulkan/device/feature.hpp"
#include "Duo2D/vulkan/device/device_type.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/traits/device_query_traits.hpp"

namespace d2d {
    template<typename... Ts> struct basic_window;
}

namespace d2d::vk {
    struct physical_device : vulkan_ptr_base<VkPhysicalDevice> {
        static result<physical_device> create(VkPhysicalDevice& device_handle, surface const& dummy_surface) noexcept;
    public:
        template<device_query Query> typename device_query_traits<Query>::return_type query(surface const& s) const noexcept = delete;

        template<> typename device_query_traits<device_query::surface_capabilites>::return_type query<device_query::surface_capabilites>(surface const& s) const noexcept;
        template<> typename device_query_traits<device_query::display_formats    >::return_type query<device_query::display_formats    >(surface const& s) const noexcept;
        template<> typename device_query_traits<device_query::present_modes      >::return_type query<device_query::present_modes      >(surface const& s) const noexcept;

        
    public:
        std::string_view name;
        device_type type = device_type::unknown;

        VkPhysicalDeviceLimits limits{};
        queue_family_idxs_t queue_family_idxs{};
        extensions_t extensions{};
        features_t features{};

    public:
        constexpr friend std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept;
    };
}


namespace d2d::vk {
    constexpr std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept {
        std::int32_t a_type_rating = a.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(a.type);
        std::int32_t b_type_rating = b.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(b.type);
        return a_type_rating <=> b_type_rating;
    }
}
