#pragma once
#include <optional>
#include <vulkan/vulkan.h>

#include "Duo2D/core/command_family.hpp"


namespace d2d {
	using render_stage_flags_t = VkPipelineStageFlagBits2;
}

namespace d2d {
	namespace render_stage {
	enum : render_stage_flags_t {
		none                           = VK_PIPELINE_STAGE_2_NONE,
		
		begin                          = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,


		draw_commands                  = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,

		index_input                    = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,
		vertex_attribute_input         = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
		vertex_shader                  = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,

		tessellation_control_shader    = VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
		tessellation_evaluation_shader = VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,

		geometry_shader                = VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,

		fragment_shader                = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		early_fragment_tests           = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
		late_fragment_tests            = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		color_attachment_output        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,


		compute_shader                 = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,


		host                           = VK_PIPELINE_STAGE_2_HOST_BIT,


		copy                           = VK_PIPELINE_STAGE_2_COPY_BIT,
		resolve                        = VK_PIPELINE_STAGE_2_RESOLVE_BIT,
		blit                           = VK_PIPELINE_STAGE_2_BLIT_BIT,
		clear                          = VK_PIPELINE_STAGE_2_CLEAR_BIT,


		end                            = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
	};
	}

	
	namespace render_stage::group {
	enum : render_stage_flags_t {
		vertex_input                   = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
		pre_rasterization              = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT,
		all_graphics                   = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		
		all_transfer                   = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
		
		all                            = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
	};
	}
}



namespace d2d::impl {
	constexpr render_stage_flags_t transfer_stages = (
		static_cast<render_stage_flags_t>(render_stage::group::all_transfer) | 
		render_stage::copy | 
		render_stage::resolve | 
		render_stage::blit | 
		render_stage::clear
	);

	constexpr std::optional<command_family_t> to_command_family(render_stage_flags_t render_stage_flags) noexcept {
		if(render_stage_flags & (render_stage::none | render_stage::begin | render_stage::end | render_stage::group::all))
			return std::nullopt;

		if(render_stage_flags & (render_stage::compute_shader))
			return command_family::compute;

		if(render_stage_flags & transfer_stages)
			return command_family::transfer;

		return command_family::graphics;
	}


	constexpr render_stage_flags_t filter_by_command_family(command_family_t CF, render_stage_flags_t render_stage_flags) noexcept {
		switch(CF) {
		case command_family::graphics:
			return render_stage_flags & ~(transfer_stages | render_stage::compute_shader);
		case command_family::compute:
			return render_stage_flags & (render_stage::compute_shader | static_cast<render_stage_flags_t>(render_stage::group::all));
		case command_family::transfer:
			return render_stage_flags & (transfer_stages | static_cast<render_stage_flags_t>(render_stage::group::all));
		default:
			return render_stage::none;
		}
	}
	
	constexpr bool has_command_family(command_family_t CF, render_stage_flags_t render_stage_flags) noexcept {
		return static_cast<bool>(filter_by_command_family(CF, render_stage_flags));
	}
}