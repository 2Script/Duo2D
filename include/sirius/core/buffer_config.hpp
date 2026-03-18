#pragma once
#include <limits>

#include <vulkan/vulkan.h>

#include "sirius/core/coupling_policy.hpp"
#include "sirius/core/memory_policy.hpp"
#include "sirius/core//shader_stage.hpp"




namespace acma {
	using buffer_usage_policy_flags_t = VkFlags;


	using dispatch_command_t = VkDispatchIndirectCommand;
	using indexed_draw_command_t = VkDrawIndexedIndirectCommand;
	using draw_command_t = VkDrawIndirectCommand;
	using draw_count_t = sl::uint32_t;
	using gpu_address_t = VkDeviceAddress;
}

namespace acma {
	namespace buffer_usage_policy {
	enum : buffer_usage_policy_flags_t {
		none,
		generic = none,
		
		uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		index   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		vertex  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

		ubo = uniform,

		num_direct_usage_polcies = impl::bit_pos(vertex) + 1,


		draw_commands     = 0b1 << (num_direct_usage_polcies + 0),
		dispatch_commands = 0b1 << (num_direct_usage_polcies + 1),
		draw_count        = 0b1 << (num_direct_usage_polcies + 2),

		num_indirect_usage_policies = impl::bit_pos(draw_count) + 1 - num_direct_usage_polcies,
		num_real_usage_policies = num_direct_usage_polcies + num_indirect_usage_policies,

		asset_heap_table = 0b1 << (num_real_usage_policies + 0),
		texture_data     = 0b1 << (num_real_usage_policies + 1),
		push_constant    = 0b1 << (num_real_usage_policies + 2),

		num_pseudo_usage_policies = impl::bit_pos(push_constant) + 1 - num_real_usage_policies,
		num_usage_policies = num_pseudo_usage_policies + num_real_usage_policies,

		max_value = ~static_cast<buffer_usage_policy_flags_t>(0) >> (std::numeric_limits<buffer_usage_policy_flags_t>::digits - (num_usage_policies)),
	};
	}
}


namespace acma {
	struct buffer_config {
		memory_policy_t memory;
		coupling_policy_t coupling;
		buffer_usage_policy_flags_t usage;
		shader_stage_flags_t stages;
		std::size_t initial_capacity_bytes = 0;
	};
}