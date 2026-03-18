#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/metaprogramming/underlying_type.hpp>
#include <streamline/containers/lookup_table.hpp>

#include <vulkan/vulkan.h>


namespace acma {
	using asset_usage_policy_t = sl::uint8_t;
}

namespace acma {
	namespace asset_usage_policy {
	enum : asset_usage_policy_t {
		//sampler asset group
		sampler,
		
		//image asset group
    	sampled_image,
    	storage_image,

		//uniform asset group
    	//uniform_data,
		//ubo = uniform_data,

		num_usage_policies,
	};
	}
}

namespace acma::vk {
	constexpr sl::array<asset_usage_policy::num_usage_policies, VkDescriptorType> descriptor_types{{
		VK_DESCRIPTOR_TYPE_SAMPLER,

		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	}};
}
