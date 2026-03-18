#pragma once
#include <bit>
#include <streamline/numeric/int.hpp>

#include <vulkan/vulkan.h>


namespace acma::impl {
	constexpr sl::uint8_t bit_pos(sl::uintmax_t val) noexcept {
		return std::countr_zero(val);
	}
}


namespace acma {
	using shader_stage_flags_t = VkShaderStageFlags;
}

namespace acma {
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