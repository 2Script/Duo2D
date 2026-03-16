#pragma once
#include <memory>
#include <streamline/numeric/int.hpp>
#include <streamline/numeric/numeric_traits.hpp>
#include <streamline/containers/array.hpp>

#include <vulkan/vulkan.h>

#include "Duo2D/arith/size.hpp"
#include "Duo2D/core/texture_usage.hpp"


namespace d2d {
	//In practice, the maximum dimension for a 2d image that any device supports is 65536 (16-bit unsigned max)
	//Since each mip level has half the size of the previous level (i.e. a bit shift right)...
	//...the absolute most amount of mip levels we can have is the number of digits in a 16-bit unsigned
	constexpr static sl::size_t max_mip_levels = sl::numeric_traits<sl::uint16_t>::digits;
}

namespace d2d {
	struct texture_info {
		sl::uint32_t                  dimensions;
        VkFormat                      format_id;
        extent3                       extent;
        sl::uint32_t                  mip_level_count;
        sl::uint32_t                  layer_count;
        VkSampleCountFlagBits         sample_count;
        VkImageTiling                 tiling;
        texture_usage                 usage;
		sl::array<max_mip_levels, sl::uoffset_t> mip_offsets;

	public:
		constexpr operator VkImageCreateInfo() const noexcept {
			return {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VkImageCreateFlags{},
				.imageType = static_cast<VkImageType>(dimensions - 1),
				.format = format_id,
				.extent = static_cast<VkExtent3D>(extent),
				.mipLevels = mip_level_count,
				.arrayLayers = layer_count,
				.samples = sample_count,
				.tiling = tiling,
				.usage = static_cast<VkImageUsageFlags>(
					(static_cast<bool>(usage) ? VK_IMAGE_USAGE_STORAGE_BIT : VK_IMAGE_USAGE_SAMPLED_BIT) |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
				),
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};
		}
	};
}