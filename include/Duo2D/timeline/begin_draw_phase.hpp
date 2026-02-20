#pragma once 
#include <streamline/numeric/int.hpp>

#include "Duo2D/arith/size.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/core/resource_table.hpp"


namespace d2d {
	struct begin_draw_phase {};
}

namespace d2d::timeline {
	template<>
	struct command<begin_draw_phase> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources> const& proc, timeline::state<N, Resources>& timeline_state, sl::empty_t) const noexcept {
			vk::command_buffer<N> const& graphics_buffer = proc.command_buffers()[proc.frame_index()][command_family::graphics];
			
			std::array<VkImageMemoryBarrier2, 2> pre_render_transitions{{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				    .srcAccessMask = VK_ACCESS_2_NONE,
				    .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				    .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				    .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				    .image = proc.swap_chain().images()[timeline_state.image_index],
				    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
				},
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				    .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				    .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				    .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				    .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				    .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				    .image = proc.depth_image().raw_image(),
				    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
				}
			}};
			graphics_buffer.pipeline_barrier({}, {}, pre_render_transitions);
			
			VkRenderingAttachmentInfo color_attachment{
			    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			    .imageView = proc.swap_chain().image_views()[timeline_state.image_index],
			    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			    .clearValue{.color{{0.0f, 0.0f, 0.0f, 1.0f}}}
			};
			VkRenderingAttachmentInfo depth_attachment {
			    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			    .imageView = proc.depth_image().view(),
			    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			    .clearValue{.depthStencil{0.0f, 0}}
			};

			const extent2 swap_chain_extent = proc.swap_chain().extent();
			graphics_buffer.begin_draw({&color_attachment, 1}, depth_attachment, {{}, swap_chain_extent}, {{}, static_cast<size2f>(swap_chain_extent)}, {{}, swap_chain_extent});
			return {};
		};
	};
}