#pragma once
#include <bit>
#include <cstdint>
#include <limits>

#include <vulkan/vulkan.h>

#include "Duo2D/core/memory_policy.hpp"
#include "Duo2D/core/render_stage.hpp"


namespace d2d::impl {
	constexpr std::uint8_t bit_pos(std::uintmax_t val) noexcept {
		return std::countr_zero(val);
	}
}


namespace d2d {
	using buffering_policy_t = bool;
	using usage_policy_flags_t = VkFlags;
	using shader_stage_flags_t = VkShaderStageFlags;



	using dispatch_command_t = VkDispatchIndirectCommand;
	using indexed_draw_command_t = VkDrawIndexedIndirectCommand;
	using draw_command_t = VkDrawIndirectCommand;
	using draw_count_t = sl::uint32_t;
	using gpu_address_t = VkDeviceAddress;
}

namespace d2d {
	namespace buffering_policy {
	enum : buffering_policy_t {
		single,
		multi
	};

	constexpr std::size_t num_buffering_policies = 2;
	}


	namespace usage_policy {
	enum : usage_policy_flags_t {
		none,


		sampler                            = 0b1 << VK_DESCRIPTOR_TYPE_SAMPLER,
		combined_sampled_image_and_sampler = 0b1 << VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		sampled_image                      = 0b1 << VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		storage_image                      = 0b1 << VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,

		uniform = 0b1 << VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		storage = 0b1 << VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		ubo  = uniform,
		ssbo = storage,

		num_sampler_descriptor_based_usage_policies = impl::bit_pos(storage_image) + 1,
		num_descriptor_based_usage_policies = impl::bit_pos(storage) + 1,
		

		index   = 0b1 << (num_descriptor_based_usage_policies + 0),
		vertex  = 0b1 << (num_descriptor_based_usage_policies + 1),
		generic = vertex,

		num_buffer_based_usage_policies = impl::bit_pos(generic) + 1 - num_descriptor_based_usage_policies,
		num_direct_usage_polcies = num_buffer_based_usage_policies + num_descriptor_based_usage_policies,


		draw_commands     = 0b1 << (num_direct_usage_polcies + 0),
		dispatch_commands = 0b1 << (num_direct_usage_polcies + 1),
		draw_count        = 0b1 << (num_direct_usage_polcies + 2),

		num_indirect_usage_policies = impl::bit_pos(draw_count) + 1 - num_direct_usage_polcies,
		num_real_usage_policies = num_direct_usage_polcies + num_indirect_usage_policies,


		push_constant     = 0b1 << (num_real_usage_policies + 0),

		num_pseudo_usage_policies = impl::bit_pos(push_constant) + 1 - num_real_usage_policies,
		num_usage_policies = num_pseudo_usage_policies + num_real_usage_policies,

		max_value = ~static_cast<usage_policy_flags_t>(0) >> (std::numeric_limits<usage_policy_flags_t>::digits - (num_usage_policies)),
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
	struct resource_config {
		memory_policy_t memory;
		buffering_policy_t buffering;
		usage_policy_flags_t usage;
		shader_stage_flags_t stages;
		std::size_t initial_capacity_bytes = 0;
	};
}