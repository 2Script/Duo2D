#pragma once
#include "sirius/vulkan/memory/device_allocation_segment.hpp"
#include <numeric>
#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/address_of.hpp>
#include <streamline/functional/invoke.hpp>

#include <vulkan/vulkan.h>

#include "sirius/core/error.hpp"


namespace acma::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    result<device_allocation_segment<I, N, BufferConfigs, RenderProcessT>> device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::create(
		std::shared_ptr<logical_device> logi_device,
		std::size_t initial_capacity,
		std::size_t initial_size
	) noexcept {
        device_allocation_segment<I, N, BufferConfigs, RenderProcessT> ret{};
        ret.allocated_bytes = initial_capacity;
		ret.data_bytes = initial_size;
		ret.desired_bytes = initial_size;
		ret.flags = 0;

		constexpr static buffer_usage_policy_flags_t usage = config.usage;
		constexpr static VkFlags all_direct_flags  = (~static_cast<VkFlags>(0) >> (std::numeric_limits<VkFlags>::digits - (buffer_usage_policy::num_direct_usage_polcies)));
		constexpr static VkFlags all_indirect_flags = (~static_cast<VkFlags>(0) >> (std::numeric_limits<VkFlags>::digits - (buffer_usage_policy::num_indirect_usage_policies))) << buffer_usage_policy::num_direct_usage_polcies;

		if constexpr(usage & all_direct_flags)
			ret.flags |= (usage & all_direct_flags);
		if constexpr (usage & all_indirect_flags)
			ret.flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if constexpr (usage & buffer_usage_policy::asset_heap_table)
			ret.flags |= VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT;
		

		ret.flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;


		for(sl::index_t i = 0; i < ret.buffs.size(); ++i) {
			ret.buffs[i] = buffer_ptr_type{logi_device};
			VkBufferCreateInfo buffer_create_info{
    		    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            	.size = initial_capacity,
    		    .usage = ret.flags,
    		    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    		};

    		__D2D_VULKAN_VERIFY(vkCreateBuffer(*logi_device, &buffer_create_info, nullptr, &ret.buffs[i]));
		}
        return ret;
    }
}



namespace acma::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    constexpr sl::index_t device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>::current_buffer_index() const noexcept {
		return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
	}
}



namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::
	reserve(sl::size_t new_capacity_bytes) noexcept {
		if(new_capacity_bytes <= this->capacity_bytes()) 
			return {};
		
		this->desired_bytes = new_capacity_bytes;
		return static_cast<RenderProcessT&>(*this).realloc(sl::index_constant<I>);
	}
}


namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr void    impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::
	clear() noexcept {
		this->data_bytes = 0;
	}
	

	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::
	resize(sl::size_t count_bytes) noexcept {
		RESULT_VERIFY(reserve(count_bytes));
		this->data_bytes = count_bytes;
		return {};
	}

	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::
	try_resize(sl::size_t count_bytes) noexcept {
		if(count_bytes > this->capacity_bytes())
			return errc::not_enough_memory;
		this->data_bytes = count_bytes;
		return {};
	}
}


namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
	requires(sl::traits::is_constructible_from_v<T, T&&> && config.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));
		
		return push_to(old_size, sl::forward<T>(t));
	}

	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	try_push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
	requires(sl::traits::is_constructible_from_v<T, T&&>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));
		
		return push_to(old_size, sl::forward<T>(t));
	}
}

namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...> && config.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));
		
		return emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}

	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	try_emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));

		return emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}
}

namespace acma::vk {
    template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
    constexpr result<void>
		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	push_to(sl::uoffset_t dst_offset, T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>) {
		std::byte* dst = this->data();
		new (dst + dst_offset) sl::remove_cvref_t<T>(sl::forward<T>(t));
		return {};
	}

    template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
    constexpr result<void>
		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	emplace_to(sl::uoffset_t dst_offset, Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>) {
		std::byte* dst = this->data();
		new (dst + dst_offset) sl::remove_cvref_t<T>(sl::forward<Args>(args)...);
		return {};
	}
}



namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	push_back(texture_view t) noexcept {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + t.bytes.size_bytes()));
		const sl::uoffset_t offset = texture_data_infos.empty() ? 0 : (texture_data_infos.back().offset + texture_data_infos.back().size);
		texture_data_infos.push_back(texture_data_info{t, offset, t.bytes.size_bytes()});
		
		std::memcpy(this->data() + offset, t.bytes.data(), t.bytes.size_bytes());
		return {};
	}

	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
	try_push_back(texture_view t) noexcept {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + t.bytes.size_bytes()));
		const sl::uoffset_t offset = texture_data_infos.empty() ? 0 : (texture_data_infos.back().offset + texture_data_infos.back().size);
		texture_data_infos.push_back(texture_data_info{t, offset, t.bytes.size_bytes()});
		
		std::memcpy(this->data() + offset, t.bytes.data(), t.bytes.size_bytes());
		return {};
	}
}



// namespace acma::vk {
// 	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	template<asset_heap_config Config, sl::index_t... Js>
// 	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
// 	emplace(
// 		asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 		uniform_buffer_segment<Js> const&... uniform_buffs
// 	) noexcept
// 	requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...) {
// 		return write_descriptors(sl::true_constant, asset_heap, uniform_buffs...);
// 	}


// 	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	template<asset_heap_config Config, sl::index_t... Js>
// 	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
// 	try_emplace(
// 		asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 		uniform_buffer_segment<Js> const&... uniform_buffs
// 	) noexcept
// 	requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...) {
// 		return write_descriptors(sl::false_constant, asset_heap, uniform_buffs...);
// 	}
// }

// namespace acma::vk {
// 	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	template<asset_heap_config Config, sl::index_t... Js>
// 	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
// 	emplace(
// 		uniform_buffer_segment<Js> const&... uniform_buffs
// 	) noexcept
// 	requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...) {
// 		return write_descriptors(sl::true_constant, {}, uniform_buffs...);
// 	}


// 	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	template<asset_heap_config Config, sl::index_t... Js>
// 	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
// 	try_emplace(
// 		uniform_buffer_segment<Js> const&... uniform_buffs
// 	) noexcept
// 	requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...) {
// 		return write_descriptors(sl::false_constant, {}, uniform_buffs...);
// 	}
// }


// namespace acma::vk {
// 		template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	template<bool ForceResize, asset_heap_config Config, sl::index_t... Js>
// 	constexpr result<void>    device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::
// 	write_descriptors(
// 		sl::bool_constant_type<ForceResize>,
// 		asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 		uniform_buffer_segment<Js> const&... uniform_buffs
// 	) noexcept {
// 		const physical_device* phys_device_ptr = static_cast<RenderProcessT const&>(*this).physical_device_ptr();
// 		const sl::array<descriptor_heap::num_descriptor_heaps, descriptor_heap_info> descriptor_heap_infos = phys_device_ptr->descriptor_heap_infos;
// 		const sl::array<asset_group::num_asset_groups, asset_group_info> asset_group_infos{{
// 			{{phys_device_ptr->descriptor_infos[asset_group::sampler]}, asset_heap._sampler_infos.size()},

// 			{{phys_device_ptr->descriptor_infos[asset_group::image]}, asset_heap._images[0].size()},
// 			{{phys_device_ptr->descriptor_infos[asset_group::uniform]}, sizeof...(Js)}
// 		}};


// 		const sl::array<asset_group::num_asset_groups, sl::size_t> desired_asset_group_sizes = sl::universal::make_deduced<sl::generic::array>(asset_group_infos, 
// 			[](asset_group_info const& info, auto) noexcept {
// 				if(info.count == 0) return 0;
// 				return ((info.count - 1) * sl::aligned_to(info.size, info.alignment)) + info.size;
// 			}
// 		);


// 		const sl::array<descriptor_heap::num_descriptor_heaps, sl::size_t> heap_sizes{{
// 			sl::aligned_to(
// 				(
// 					sl::aligned_to(descriptor_heap_infos[descriptor_heap::sampler].minimum_reserved_bytes, asset_group_infos[asset_group::sampler].alignment) + 
// 					desired_asset_group_sizes[asset_group::sampler]
// 				),
// 				descriptor_heap_infos[descriptor_heap::resource].alignment
// 			),
// 			(
// 				sl::aligned_to(descriptor_heap_infos[descriptor_heap::resource].minimum_reserved_bytes, asset_group_infos[asset_group::image].alignment) + 
// 				sl::aligned_to(desired_asset_group_sizes[asset_group::image], asset_group_infos[asset_group::uniform].alignment) + 
// 				desired_asset_group_sizes[asset_group::uniform]
// 			)
// 		}};
		

// 		const sl::size_t new_size = std::accumulate(heap_sizes.cbegin(), heap_sizes.cend(), 0);
// 		if constexpr(ForceResize)
// 			RESULT_VERIFY(this->resize(new_size))
// 		else
// 			RESULT_VERIFY(this->try_resize(new_size))

// 		heap_offsets = {{
// 			0, heap_sizes[descriptor_heap::sampler],
// 		}};

// 		const sl::array<asset_group::num_asset_groups, sl::size_t> descriptor_write_offsets{{
// 			sl::aligned_to(
// 				descriptor_heap_infos[descriptor_heap::sampler].minimum_reserved_bytes,
// 				asset_group_infos[asset_group::sampler].alignment
// 			),

// 			heap_sizes[descriptor_heap::sampler] + sl::aligned_to(descriptor_heap_infos[descriptor_heap::resource].minimum_reserved_bytes, asset_group_infos[asset_group::image].alignment),
// 			heap_sizes[descriptor_heap::sampler] + heap_sizes[descriptor_heap::resource] - desired_asset_group_sizes[asset_group::uniform]
// 		}};


// 		const std::shared_ptr<logical_device> logi_device_ptr = static_cast<RenderProcessT const&>(*this).logical_device_ptr();
		
// 		//Write samplers
// 		std::unique_ptr<VkHostAddressRangeEXT[]> sampler_descriptor_write_ranges = std::make_unique_for_overwrite<VkHostAddressRangeEXT[]>(asset_group_infos[asset_group::sampler].count);
// 		{
// 		const sl::index_t i = asset_group::sampler;
// 		for(sl::index_t j = 0; j < asset_group_infos[i].count; ++j) {
// 			sampler_descriptor_write_ranges[j] = {
// 				.address = this->data() + 
// 					descriptor_write_offsets[i] + 
// 					(j * sl::aligned_to(asset_group_infos[i].size, asset_group_infos[i].alignment)),
// 				.size = asset_group_infos[i].size
// 			};
// 		}
// 		}

// 		__D2D_VULKAN_VERIFY(sl::invoke(logi_device_ptr->vulkan_functions()[sl::index_constant<extended_functions::vkWriteSamplerDescriptors>], 
// 			*logi_device_ptr,
// 			asset_group_infos[asset_group::sampler].count,
// 			asset_heap._sampler_infos.data(),
// 			sampler_descriptor_write_ranges.get()
// 		));


// 		std::unique_ptr<VkResourceDescriptorInfoEXT[]> resource_descriptor_infos = std::make_unique_for_overwrite<VkImageViewCreateInfo[]>(
// 			asset_group_infos[asset_group::image].count + asset_group_infos[asset_group::uniform].count
// 		);

// 		//Create image descriptors
// 		std::unique_ptr<VkImageViewCreateInfo[]> image_view_infos = std::make_unique_for_overwrite<VkImageViewCreateInfo[]>(asset_group_infos[asset_group::image].count);
// 		std::unique_ptr<VkImageDescriptorInfoEXT[]> image_descriptor_infos = std::make_unique_for_overwrite<VkImageDescriptorInfoEXT[]>(asset_group_infos[asset_group::image].count);
		
// 		for(sl::size_t i = 0; i < asset_group_infos[asset_group::image].count; ++i) {
// 			image const& current_image = asset_heap._images[this->current_buffer_index()][i];
// 			image_view_infos[i] = {
// 				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
// 				.pNext = nullptr,
// 				.flags = 0,
// 				.image = current_image,
// 				.viewType = current_image.layer_count() > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
// 				.format = current_image.format_id(),
// 				.components{},
// 				.subresourceRange{
// 					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
// 					.baseMipLevel = 0,
// 					.levelCount = current_image.mip_level_count(),
// 					.baseArrayLayer = 0,
// 					.layerCount = current_image.layer_count()
// 				}
// 			};
// 			image_descriptor_infos[i] = {
// 				.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
// 				.pNext = nullptr,
// 				.pView = &image_view_infos[i],
// 				.layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
// 			};
// 			resource_descriptor_infos[i] = {
// 				.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
// 		    	.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
// 		    	.data{.pImage = &image_descriptor_infos[i]}
// 			};
// 		}
		
// 		//Create uniform buffer descriptors
// 		const sl::array<sizeof...(Js), VkDeviceAddressRangeEXT> uniform_descriptor_infos{{
// 			{uniform_buffs.gpu_address(), uniform_buffs.size_bytes()}...
// 		}};

// 		for(sl::size_t i = 0; i < asset_group_infos[asset_group::uniform].count; ++i) {
// 			resource_descriptor_infos[i + asset_group_infos[asset_group::image].count] = {
// 				.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
// 		    	.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
// 		    	.data{.pAddressRange = &uniform_descriptor_infos[i]}
// 			};
// 		}

// 		const sl::array<asset_group::num_asset_groups, sl::size_t> resource_descriptor_count{{
// 			0, asset_group_infos[asset_group::image].count, asset_group_infos[asset_group::uniform].count
// 		}};


// 		std::unique_ptr<VkHostAddressRangeEXT[]> resource_descriptor_write_ranges = std::make_unique_for_overwrite<VkHostAddressRangeEXT[]>(
// 			asset_group_infos[asset_group::image].count + asset_group_infos[asset_group::uniform].count
// 		);
// 		for(sl::index_t i = asset_group::image; i <= asset_group::uniform; ++i) {
// 			for(sl::index_t j = resource_descriptor_count[i - 1]; j < resource_descriptor_count[i]; ++j) {
// 				resource_descriptor_write_ranges[j] = {
// 					.address = this->data() + 
// 						descriptor_write_offsets[i] + 
// 						((j - resource_descriptor_count[i - 1]) * sl::aligned_to(asset_group_infos[i].size, asset_group_infos[i].alignment)),
// 					.size = asset_group_infos[i].size
// 				};
// 			}
// 		}
		
// 		__D2D_VULKAN_VERIFY(sl::invoke(logi_device_ptr->vulkan_functions()[sl::index_constant<extended_functions::vkWriteResourceDescriptors>], 
// 			*logi_device_ptr,
// 			asset_group_infos[asset_group::image].count + asset_group_infos[asset_group::uniform].count,
// 			resource_descriptor_infos.get(),
// 			resource_descriptor_write_ranges.get()
// 		));

// 		return {};
// 	}
// }