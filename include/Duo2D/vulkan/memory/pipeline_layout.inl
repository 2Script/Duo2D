#pragma once
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"

#include <streamline/functional/functor/forward_construct.hpp>
#include <streamline/functional/functor/invoke_each.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>

namespace d2d::vk {
    template<shader_stage_flags_t Stages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
	template<typename RenderProcessT>
    result<pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>>
		pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>::
	create(
		std::shared_ptr<logical_device> logi_device,
		RenderProcessT const& proc
	) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = logi_device;


		using push_constant_usage_filtered_sequence = sl::filtered_sequence_t<sl::remove_cvref_t<decltype(T::buffers)>, []<buffer_key_t K>(buffer_key_constant_type<K>) noexcept { 
			constexpr buffer_config cfg = BufferConfigs[K];
			return cfg.usage == buffer_usage_policy::push_constant && (cfg.stages & Stages);
		}>;
		constexpr auto push_constant_ranges = sl::make<sl::array<push_constant_usage_filtered_sequence::size(), VkPushConstantRange>>(
			BufferConfigs, 
			[](auto pair, auto) noexcept -> VkPushConstantRange {
				const buffer_config cfg = pair[sl::second_constant];
				return {cfg.stages, 0, static_cast<std::uint32_t>(cfg.initial_capacity_bytes)};
			},
			push_constant_usage_filtered_sequence{}
		);



		{
		ret.uniform_set_layout = descriptor_set_layout_type{logi_device};
		const sl::array<uniform_buffer_count, VkDescriptorSetLayoutBinding> bindings = sl::make<sl::array<uniform_buffer_count, VkDescriptorSetLayoutBinding>>(
			T::uniform_buffer_order,
			[](buffer_key_t key, auto) noexcept {
				return VkDescriptorSetLayoutBinding{
					.binding = uniform_buffer_binding_indices[key],
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = BufferConfigs[key].stages,
					.pImmutableSamplers = nullptr,
				};
			},
			uniform_stage_filtered_sequence{}
		);
		const VkDescriptorSetLayoutCreateInfo set_layout_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data(),
		};

		__D2D_VULKAN_VERIFY(vkCreateDescriptorSetLayout(*logi_device, &set_layout_info, nullptr, &ret.uniform_set_layout));
		}


		using set_layout_handle_array_type = sl::array<asset_heap_count * asset_usage_policy::num_usage_policies + 1, VkDescriptorSetLayout>;
		constexpr auto create_handles = []<sl::index_t I>(
			set_layout_handle_array_type& arr,
			sl::index_t& out_idx,
			RenderProcessT const& proc,
			sl::index_constant_type<I>
		) noexcept {
			constexpr asset_heap_key_t key = sl::universal::get<I>(T::asset_heaps);
			for(asset_usage_policy_t j = 0; j < asset_usage_policy::num_usage_policies; ++j) 
				arr[1 + (out_idx * asset_usage_policy::num_usage_policies) + j] = sl::universal::get<key>(proc)._descriptor_set_layouts[j];
			++out_idx;
		};
		sl::index_t set_layout_handle_idx = 0;
		set_layout_handle_array_type set_layout_handles{};
		sl::invoke(
			sl::functor::invoke_each<create_handles>{},
			asset_heap_stage_filtered_sequence{},
			set_layout_handles, set_layout_handle_idx, proc
		);
		set_layout_handles[0] = ret.uniform_set_layout;
		
		
		VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = set_layout_handles.size(),
			.pSetLayouts = set_layout_handles.data(),
			.pushConstantRangeCount = push_constant_ranges.size(),
			.pPushConstantRanges = push_constant_ranges.data(),
        };
        __D2D_VULKAN_VERIFY(vkCreatePipelineLayout(*logi_device, &pipeline_layout_create_info, nullptr, &ret.handle));

        return ret;
    }
}
