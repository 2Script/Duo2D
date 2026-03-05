#pragma once
#include <bit>
#include <cstdint>
#include <limits>

#include <streamline/containers/array.hpp>
#include <vulkan/vulkan.h>

#include "Duo2D/core/coupling_policy.hpp"
#include "Duo2D/core/memory_policy.hpp"
#include "Duo2D/core/render_stage.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"


namespace d2d {
	using asset_usage_policy_t = sl::uint8_t;
	using asset_group_t = sl::uint8_t;
}

namespace d2d {
	namespace asset_usage_policy {
	enum : asset_usage_policy_t {
		//sampler asset group
		sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		
		//image asset group
    	sampled_image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    	storage_image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    	//uniform_texel = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    	//storage_texel = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,

		//uniform asset group
    	uniform_data = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		ubo = uniform_data,

		num_usage_policies = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER + 1,
	};
	}

	namespace asset_group {
	enum : asset_group_t {
		sampler,
		image,
		uniform,

		num_asset_groups,
	};
	}
}


namespace d2d {
	struct asset_heap_config {
		memory_policy_t image_memory;
		coupling_policy_t image_coupling;
	};
}