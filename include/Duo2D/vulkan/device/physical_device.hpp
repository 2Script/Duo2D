#pragma once
#include <string_view>
#include <compare>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <streamline/containers/array.hpp>

#include "Duo2D/core/command_family.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/device/device_query.hpp"
#include "Duo2D/vulkan/device/device_query_traits.hpp"
#include "Duo2D/vulkan/device/queue_family_info.hpp"
#include "Duo2D/vulkan/device/extension.hpp"
#include "Duo2D/vulkan/device/feature.hpp"
#include "Duo2D/vulkan/device/device_type.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/memory/asset_group.hpp"
#include "Duo2D/vulkan/memory/descriptor_heap.hpp"
#include "Duo2D/vulkan/memory/asset_usage_policy.hpp"


namespace d2d::vk {
	struct asset_group_info {
		sl::size_t size;
		sl::size_t alignment;
	};

	struct descriptor_heap_info {
		sl::size_t minimum_reserved_bytes;
		sl::size_t alignment;
		sl::size_t max_size;
	};
}

namespace d2d::vk {
    struct physical_device : vulkan_ptr_base<VkPhysicalDevice> {
        static result<physical_device> create(VkPhysicalDevice& device_handle, std::shared_ptr<vk::instance> instance, bool prefer_synchronous_rendering, bool window_capability) noexcept;
    public:
        template<device_query Query> typename device_query_traits<Query>::return_type query(surface const& s) const noexcept = delete;

        template<> typename device_query_traits<device_query::surface_capabilites>::return_type query<device_query::surface_capabilites>(surface const& s) const noexcept;
        template<> typename device_query_traits<device_query::display_formats    >::return_type query<device_query::display_formats    >(surface const& s) const noexcept;
        template<> typename device_query_traits<device_query::present_modes      >::return_type query<device_query::present_modes      >(surface const& s) const noexcept;

        
    public:
        std::string_view name;
        device_type type = device_type::unknown;
        extensions_t extensions{};
        features_t features{};
        VkPhysicalDeviceLimits limits{};
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> descriptor_count_limits{};
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> per_stage_descriptor_count_limits{};

		sl::size_t max_push_data_bytes;
		sl::array<asset_group::num_asset_groups, asset_group_info> asset_group_infos{};
		sl::array<descriptor_heap::num_descriptor_heaps, descriptor_heap_info> descriptor_heap_infos{};
        sl::array<command_family::num_families, queue_family_info> queue_family_infos{};

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
