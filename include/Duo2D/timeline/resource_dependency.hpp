#pragma once
#include "Duo2D/core/memory_operation.hpp"
#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/timeline/state.hpp"


namespace d2d {
	template<
		command_family_t ExecutionCommandFamily,
		render_stage_flags_t SourceStages, memory_operation_t SourceMemoryOp,
		render_stage_flags_t DestinationStages, memory_operation_t DestinationMemoryOp,
		typename KeySeq
	>
	struct resource_dependency {};
}


namespace d2d::timeline {
	template<
		command_family_t ExecutionCommandFamily,
		render_stage_flags_t SourceStages, memory_operation_t SourceMemoryOp,
		render_stage_flags_t DestinationStages, memory_operation_t DestinationMemoryOp,
		resource_key_t... ResourceKeys
	>
	struct command<resource_dependency<ExecutionCommandFamily, SourceStages, SourceMemoryOp, DestinationStages, DestinationMemoryOp, resource_key_sequence_type<ResourceKeys...>>> {
		template<sl::size_t N, resource_table<N> Resources>
		constexpr result<void> operator()(render_process<N, Resources> const& proc, timeline::state<N, Resources>&, sl::empty_t) const noexcept {
			constexpr std::optional<command_family_t> src_command_family = ::d2d::impl::to_command_family(SourceStages);
			constexpr std::optional<command_family_t> dst_command_family = ::d2d::impl::to_command_family(DestinationStages);
			constexpr bool is_inter_command = src_command_family.has_value() && dst_command_family.has_value() && *src_command_family != *dst_command_family;
			const bool different_queues = is_inter_command && proc.physical_device_ptr()->queue_family_infos[*src_command_family].index != proc.physical_device_ptr()->queue_family_infos[*dst_command_family];

			vk::command_buffer<N> const& cmd_buff = proc.command_buffers()[ExecutionCommandFamily];
			
			//If it's an inter-command dependency and the queues are the same, do nothing (submit<> will handle the syncronization)
			if constexpr(is_inter_command)
				if(!different_queues)
					return {};


			std::array<VkBufferMemoryBarrier2, sizeof...(ResourceKeys)> barriers{{
				VkBufferMemoryBarrier2{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.srcStageMask  = (is_inter_command && ExecutionCommandFamily != *src_command_family) ? VK_PIPELINE_STAGE_2_NONE : SourceStages,
					.srcAccessMask = (is_inter_command && ExecutionCommandFamily != *src_command_family) ? VK_ACCESS_2_NONE : SourceMemoryOp,
					.dstStageMask  = (is_inter_command && ExecutionCommandFamily != *dst_command_family) ? VK_PIPELINE_STAGE_2_NONE : DestinationStages,
					.dstAccessMask = (is_inter_command && ExecutionCommandFamily != *dst_command_family) ? VK_ACCESS_2_NONE : DestinationMemoryOp,
					.srcQueueFamilyIndex = different_queues ? *src_command_family : VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = different_queues ? *dst_command_family : VK_QUEUE_FAMILY_IGNORED,
					.buffer = proc[resource_key_constant_type<ResourceKeys>{}],
					.offset = 0,
					.size = proc[resource_key_constant_type<ResourceKeys>{}].size_bytes()
				}...
			}};
			
			cmd_buff.pipeline_barrier({}, barriers, {});

			return {};
		};
	};
}