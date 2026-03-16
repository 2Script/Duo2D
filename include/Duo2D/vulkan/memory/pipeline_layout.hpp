#pragma once
#include <streamline/functional/functor/identity_index.hpp>

#include <vulkan/vulkan_core.h>

#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/memory/asset_heap_allocation.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipelineLayout);

namespace d2d::vk {
    template<shader_stage_flags_t Stages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline_layout : vulkan_ptr<VkPipelineLayout, vkDestroyPipelineLayout> {
		template<typename RenderProcessT>
        static result<pipeline_layout> create(
			std::shared_ptr<logical_device> logi_device,
			RenderProcessT const& proc
		) noexcept;
	
	private:
		using asset_heap_stage_filtered_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<decltype(T::asset_heaps)::size()>, 
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept {
				constexpr buffer_key_t key = sl::universal::get<I>(T::asset_heaps);
				return static_cast<bool>(AssetHeapConfigs[key].stages & Stages);
			}
		>;
		constexpr static sl::size_t asset_heap_count = asset_heap_stage_filtered_sequence::size();
		constexpr static sl::array<asset_heap_count, shader_stage_flags_t> set_layout_stages = 
			sl::universal::make<sl::array<asset_heap_count, shader_stage_flags_t>>(
				T::asset_heaps, 
				[](asset_heap_key_t key, auto) noexcept -> shader_stage_flags_t {
					return AssetHeapConfigs[key].stages;
				}
			
		);


	private:
		using uniform_stage_filtered_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<decltype(T::uniform_buffer_order)::size()>,
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept { 
				constexpr buffer_key_t key = sl::universal::get<I>(T::uniform_buffer_order);
				return static_cast<bool>(BufferConfigs[key].stages & Stages);
			}
		>;
		constexpr static sl::size_t uniform_buffer_count = uniform_stage_filtered_sequence::size();

	public:
		constexpr static sl::lookup_table<uniform_buffer_count, buffer_key_t, sl::uint32_t> uniform_buffer_binding_indices = 
			sl::universal::make<sl::lookup_table<uniform_buffer_count, buffer_key_t, sl::uint32_t>>(
				T::uniform_buffer_order,
				sl::functor::identity{}, sl::functor::identity_index{},
				uniform_stage_filtered_sequence{}
			);

	private:
		constexpr static sl::size_t max_push_descriptor_count = 32;
		
	private:
		descriptor_set_layout_type uniform_set_layout;
    };
}

#include "Duo2D/vulkan/memory/pipeline_layout.inl"