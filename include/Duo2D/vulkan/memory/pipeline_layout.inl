#pragma once
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"

namespace d2d::vk {
    template<typename T, sl::size_t N, resource_table<N> Resources>
    result<pipeline_layout<T, N, Resources>> pipeline_layout<T, N, Resources>::create(std::shared_ptr<logical_device> device) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = device;


		using push_constant_usage_filtered_sequence = sl::filtered_sequence_t<sl::index_sequence_of_length_type<N>, []<sl::index_t I>(sl::index_constant_type<I>){ 
			return sl::universal::get<sl::second_constant>(*std::next(Resources.begin(), I)).usage == usage_policy::push_constant;
		}>;
		constexpr auto push_constant_ranges = sl::make<sl::array<push_constant_usage_filtered_sequence::size(), VkPushConstantRange>>(Resources, [](auto pair, auto) noexcept -> VkPushConstantRange {
			const resource_config cfg = pair[sl::second_constant];
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
