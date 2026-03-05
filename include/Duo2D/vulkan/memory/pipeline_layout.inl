#pragma once
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"

#include <streamline/metaprogramming/integer_sequence.hpp>

namespace d2d::vk {
    template<shader_stage_flags_t Stages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    result<pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>> pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>::create(std::shared_ptr<logical_device> device) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = device;


		using push_constant_usage_filtered_sequence = sl::filtered_sequence_t<sl::remove_cvref_t<decltype(T::buffers)>, []<buffer_key_t K>(buffer_key_constant_type<K>) noexcept { 
			constexpr buffer_config cfg = BufferConfigs[K];
			return cfg.usage == buffer_usage_policy::push_constant && (cfg.stages & Stages);
		}>;
		constexpr auto push_constant_ranges = sl::make<sl::array<push_constant_usage_filtered_sequence::size(), VkPushConstantRange>>(BufferConfigs, [](auto pair, auto) noexcept -> VkPushConstantRange {
			const buffer_config cfg = pair[sl::second_constant];
			return {cfg.stages, 0, static_cast<std::uint32_t>(cfg.initial_capacity_bytes)};
		}, push_constant_usage_filtered_sequence{});

		VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
			.pushConstantRangeCount = push_constant_ranges.size(),
			.pPushConstantRanges = push_constant_ranges.data(),
        };
        __D2D_VULKAN_VERIFY(vkCreatePipelineLayout(*device, &pipeline_layout_create_info, nullptr, &ret.handle));

        return ret;
    }
}
