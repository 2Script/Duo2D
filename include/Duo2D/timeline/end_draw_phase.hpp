#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/timeline/event.hpp"


namespace d2d {
	struct end_draw_phase : timeline::event {
		constexpr static command_family_t family = command_family::graphics;
	};
}

namespace d2d::timeline {
	template<>
	struct command<end_draw_phase> {
		template<sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(render_process<N, Resources, CommandGroupCount> const& proc, timeline::state<N, Resources, CommandGroupCount>& timeline_state, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			vk::command_buffer<N> const& graphics_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			
			graphics_buffer.end_draw();

			VkImageMemoryBarrier2 post_render_transition{
			    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			    .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			    .dstStageMask = proc.has_dedicated_present_queue() ? 
					VK_PIPELINE_STAGE_2_NONE : 
					VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			    .dstAccessMask = VK_ACCESS_2_NONE,
			    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.srcQueueFamilyIndex = proc.has_dedicated_present_queue() ? 
					proc.physical_device_ptr()->queue_family_infos[command_family::graphics].index : 
					VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = proc.has_dedicated_present_queue() ? 
					proc.physical_device_ptr()->queue_family_infos[command_family::present].index : 
					VK_QUEUE_FAMILY_IGNORED,
			    .image = proc.swap_chain().images()[timeline_state.image_index],
			    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
			};

			graphics_buffer.pipeline_barrier({}, {}, {&post_render_transition, 1});
			return {};
		};
	};
}