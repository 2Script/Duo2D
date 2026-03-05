#pragma once
#include <bit>
#include <cstdint>
#include <limits>

#include <vulkan/vulkan.h>

#include "Duo2D/core/coupling_policy.hpp"
#include "Duo2D/core/memory_policy.hpp"
#include "Duo2D/core/render_stage.hpp"


namespace d2d::impl {
	constexpr std::uint8_t bit_pos(std::uintmax_t val) noexcept {
		return std::countr_zero(val);
	}
}


namespace d2d {
	using buffer_usage_policy_flags_t = VkFlags;
	using shader_stage_flags_t = VkShaderStageFlags;


	using dispatch_command_t = VkDispatchIndirectCommand;
	using indexed_draw_command_t = VkDrawIndexedIndirectCommand;
	using draw_command_t = VkDrawIndirectCommand;
	using draw_count_t = sl::uint32_t;
	using gpu_address_t = VkDeviceAddress;
}

namespace d2d {
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

		push_constant = 0b1 << (num_real_usage_policies + 1),

		num_pseudo_usage_policies = impl::bit_pos(push_constant) + 1 - num_real_usage_policies,
		num_usage_policies = num_pseudo_usage_policies + num_real_usage_policies,

		max_value = ~static_cast<buffer_usage_policy_flags_t>(0) >> (std::numeric_limits<buffer_usage_policy_flags_t>::digits - (num_usage_policies)),
	};
	}


	namespace shader_stage {
	enum : shader_stage_flags_t {
		vertex            = VK_SHADER_STAGE_VERTEX_BIT,
		tessellation_ctrl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		tessellation_eval = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		geometry          = VK_SHADER_STAGE_GEOMETRY_BIT,
		fragment          = VK_SHADER_STAGE_FRAGMENT_BIT,
		compute           = VK_SHADER_STAGE_COMPUTE_BIT,

		all_graphics = VK_SHADER_STAGE_ALL_GRAPHICS,
		all = VK_SHADER_STAGE_ALL,


		num_shader_stages = impl::bit_pos(compute) - impl::bit_pos(vertex) + 1
	};
	}
}


namespace d2d {
	struct buffer_config {
		memory_policy_t memory;
		coupling_policy_t coupling;
		buffer_usage_policy_flags_t usage;
		shader_stage_flags_t stages;
		std::size_t initial_capacity_bytes = 0;
	};
}