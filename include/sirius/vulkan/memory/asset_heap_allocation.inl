#pragma once
#include "sirius/vulkan/memory/asset_heap_allocation.hpp"

#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/forward_construct.hpp>

#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/timeline/dedicated_command_group.hpp"


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	result<asset_heap_allocation<I, Config, RenderProcessT>>
		asset_heap_allocation<I, Config, RenderProcessT>::
	create(
		std::shared_ptr<logical_device> logi_device,
		physical_device* phys_device//,
		//std::shared_ptr<command_pool> transfer_command_pool
	) noexcept {
        asset_heap_allocation ret{}; 
        RESULT_VERIFY(ret.initialize(logi_device, phys_device));//, transfer_command_pool));

		constexpr static sl::uint32_t stage_count = std::popcount(Config.stages);
		for(asset_usage_policy_t j = 0; j < asset_usage_policy::num_usage_policies; ++j) {
			ret._descriptor_set_layouts[j] = descriptor_set_layout_type{logi_device};

			VkDescriptorSetLayoutBinding set_layout_binding{
				.binding = 0,
				.descriptorType = vk::descriptor_types[j],
				.descriptorCount = std::min(std::min(
					phys_device->descriptor_count_limits[j] / stage_count,
					phys_device->per_stage_descriptor_count_limits[j]),
					phys_device->limits.maxPerStageResources / (stage_count * asset_usage_policy::num_usage_policies)
				),
				.stageFlags = Config.stages,
				.pImmutableSamplers = nullptr,
			};
			constexpr static VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
			const VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.pNext = nullptr,
				.bindingCount = 1,
				.pBindingFlags = &binding_flags
			};
			const VkDescriptorSetLayoutCreateInfo set_layout_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext = &binding_flags_info,
				.flags = VkDescriptorSetLayoutCreateFlags{},
				.bindingCount = 1,
				.pBindings = &set_layout_binding,
			};

			__D2D_VULKAN_VERIFY(vkCreateDescriptorSetLayout(*logi_device, &set_layout_info, nullptr, &ret._descriptor_set_layouts[j]));
		}

		for(sl::index_t i = 0; i < allocation_count; ++i) {
			ret._descriptor_pools[i] = descriptor_pool_type{logi_device};

			RESULT_VERIFY(ret.make_pools({}, i));
		}

		return ret;
	}
}


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<typename T>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, T&&>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, T&&>) {
		const sl::index_t alloc_idx = allocation_index();

		_sampler_infos[alloc_idx].push_back(sl::forward<T>(t));

		result<image_sampler> sampler_result = make<image_sampler>(this->logi_device_ptr, _sampler_infos[alloc_idx].back());
		if(!sampler_result.has_value()) {
			_sampler_infos[alloc_idx].pop_back();
			return sampler_result.error();
		}
		_samplers[alloc_idx].push_back(*sl::move(sampler_result));

		constexpr static asset_usage_policy_t usage = asset_usage_policy::sampler;

		{
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		++desired_descriptor_counts[usage];
		RESULT_VERIFY(resize(desired_descriptor_counts));
		}

		update_descriptors(alloc_idx);

		return {};
	}

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<typename... Args>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, Args&&...>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, Args&&...>) {
		const sl::index_t alloc_idx = allocation_index();

		_sampler_infos[alloc_idx].emplace_back(sl::forward<Args>(args)...);


		result<image_sampler> sampler_result = make<image_sampler>(this->logi_device_ptr, _sampler_infos[alloc_idx].back());
		if(!sampler_result.has_value()) {
			_sampler_infos[alloc_idx].pop_back();
			return sampler_result.error();
		}
		_samplers[alloc_idx].push_back(*sl::move(sampler_result));

		constexpr static asset_usage_policy_t usage = asset_usage_policy::sampler;
		
		{
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		++desired_descriptor_counts[usage];
		RESULT_VERIFY(try_resize(desired_descriptor_counts));
		}

		update_descriptors(alloc_idx);

		return {};
	}
}

// namespace acma::vk {
// 	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
// 	template<sl::index_t J, sl::size_t N, auto BufferIs>
// 	constexpr result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
//  	emplace_back(buffer_segment<J, N, BufferIs> const& uniform_buffer) noexcept
// 	requires((buffer_segment<J, N, BufferIs>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform) {
// 		if(uniform_buffer.size() == 0) 
// 			return {};

// 		const sl::index_t alloc_idx = allocation_index();
// 		constexpr static asset_usage_policy_t usage = asset_usage_policy::uniform_data;
		
// 		{
// 		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
// 		++desired_descriptor_counts[usage];
// 		RESULT_VERIFY(resize(desired_descriptor_counts));
// 		}

// 		_descriptor_info_vectors[alloc_idx][usage].push_back({.buffer{
// 			.buffer = static_cast<VkBuffer>(uniform_buffer),
// 			.offset = 0,
// 			.range = uniform_buffer.size_bytes()
// 		}});
// 		update_descriptors(usage, alloc_idx);

// 		return {};
// 	}

// 	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
// 	template<sl::index_t J, sl::size_t N, auto BufferIs>
// 	constexpr result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
//  	try_emplace_back(buffer_segment<J, N, BufferIs> const& uniform_buffer) noexcept
// 	requires((buffer_segment<J, N, BufferIs>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform) {
// 		if(uniform_buffer.size() == 0) 
// 			return {};

// 		const sl::index_t alloc_idx = allocation_index();
// 		constexpr static asset_usage_policy_t usage = asset_usage_policy::uniform_data;
		
// 		{
// 		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
// 		++desired_descriptor_counts[usage];
// 		RESULT_VERIFY(try_resize(desired_descriptor_counts));
// 		}

// 		_descriptor_info_vectors[alloc_idx][usage].push_back({.buffer{
// 			.buffer = static_cast<VkBuffer>(uniform_buffer),
// 			.offset = 0,
// 			.range = uniform_buffer.size_bytes()
// 		}});
// 		update_descriptors(usage, alloc_idx);

// 		return {};
// 	}
// }

namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<sl::index_t J, sl::size_t N, auto BufferIs>
	constexpr result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
 	emplace_back(buffer_segment<J, N, BufferIs> const& texture_data_buffer) noexcept
	requires((buffer_segment<J, N, BufferIs>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data) {
		if(texture_data_buffer.texture_data_infos.empty() || texture_data_buffer.size() == 0) 
			return {};

		sl::size_t new_size = data_bytes;
		RESULT_TRY_MOVE_UNSCOPED(
			std::unique_ptr<image_allocation[]> img_allocs,
			make_images(texture_data_buffer.texture_data_infos, new_size),
		_ia);

		RESULT_VERIFY(resize(new_size));

		const sl::index_t alloc_idx = allocation_index();
		const sl::size_t start_idx = _images[alloc_idx].size();
		for(sl::index_t i = 0; i < texture_data_buffer.texture_data_infos.size(); ++i)
			RESULT_VERIFY(bind(sl::move(img_allocs[i]), alloc_idx));

		RESULT_VERIFY(upload_image_data(texture_data_buffer, alloc_idx, start_idx));

		RESULT_VERIFY(initialize_image_views(alloc_idx));
		
		{
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		for(sl::index_t i = 0; i < texture_data_buffer.texture_data_infos.size(); ++i) {
			const asset_usage_policy_t usage = asset_usage_policy::sampled_image + static_cast<asset_usage_policy_t>(texture_data_buffer.texture_data_infos[i].usage);
			++desired_descriptor_counts[usage];
			_image_usages[alloc_idx].push_back(usage);
		}
		RESULT_VERIFY(resize(desired_descriptor_counts));
		}

		
		update_descriptors(alloc_idx);
		return {};
	}

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<sl::index_t J, sl::size_t N, auto BufferIs>
	constexpr result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
 	try_emplace_back(buffer_segment<J, N, BufferIs> const& texture_data_buffer) noexcept
	requires((buffer_segment<J, N, BufferIs>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data) {
		if(texture_data_buffer.texture_data_infos.empty()) 
			return {};

		sl::size_t new_size = data_bytes;
		RESULT_TRY_MOVE_UNSCOPED(
			std::unique_ptr<image_allocation[]> img_allocs,
			make_images(texture_data_buffer.texture_data_infos, new_size),
		_ia);

		RESULT_VERIFY(try_resize(new_size));

		const sl::index_t alloc_idx = allocation_index();
		const sl::size_t start_idx = _images[0].size();
		for(sl::index_t i = 0; i < img_allocs.size(); ++i)
			RESULT_VERIFY(bind(sl::move(img_allocs[i]), alloc_idx));

		RESULT_VERIFY(upload_image_data(texture_data_buffer, alloc_idx, start_idx));

		RESULT_VERIFY(initialize_image_views(alloc_idx));
		
		{
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		for(sl::index_t i = 0; i < texture_data_buffer.texture_data_infos.size(); ++i) {
			const asset_usage_policy_t usage = asset_usage_policy::sampled_image + static_cast<asset_usage_policy_t>(texture_data_buffer.texture_data_infos[i].usage);
			++desired_descriptor_counts[usage];
			_image_usages[alloc_idx].push_back(usage);
		}
		RESULT_VERIFY(try_resize(desired_descriptor_counts));
		}

		
		update_descriptors(alloc_idx);
		return {};
	}
}


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	reserve(sl::size_t new_capacity_bytes) noexcept {
		if(new_capacity_bytes <= allocated_bytes)
			return {};
		
		this->desired_bytes = new_capacity_bytes;
		return realloc(sl::index_constant<I>);
	}


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	reserve(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept {
		const sl::index_t alloc_idx = allocation_index();
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i)
			if(asset_counts[i] > _descriptor_counts[alloc_idx][i])
				return make_pools(asset_counts, alloc_idx);
		return {};
	}
}


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr void    asset_heap_allocation<I, Config, RenderProcessT>::
	clear() noexcept {
		this->data_bytes = 0;
	}
	

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	resize(sl::size_t count_bytes) noexcept {
		RESULT_VERIFY(reserve(count_bytes));
		this->data_bytes = count_bytes;
		return {};
	}

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	try_resize(sl::size_t count_bytes) noexcept {
		if(count_bytes > this->capacity_bytes())
			return errc::not_enough_memory;
		this->data_bytes = count_bytes;
		return {};
	}


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	resize(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept {
		RESULT_VERIFY(reserve(asset_counts));
		_descriptor_counts[allocation_index()] = asset_counts;
		return {};
	}

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	try_resize(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept {
		const sl::index_t alloc_idx = allocation_index();
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i)
			if(asset_counts[i] > _descriptor_counts[alloc_idx][i])
				return errc::not_enough_memory;
		_descriptor_counts[alloc_idx] = asset_counts;
		return {};
	}
}


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<std::unique_ptr<typename asset_heap_allocation<I, Config, RenderProcessT>::image_allocation[]>>
		asset_heap_allocation<I, Config, RenderProcessT>::
	make_images(std::vector<texture_data_info> const& texture_data_infos, sl::size_t& total_size_out) noexcept {
		std::unique_ptr<image_allocation[]> ret = std::make_unique_for_overwrite<image_allocation[]>(texture_data_infos.size());
		for(sl::size_t i = 0; i < texture_data_infos.size(); ++i) {
			RESULT_TRY_MOVE_UNSCOPED(image new_img, make<image>(this->logi_device_ptr, static_cast<VkImageCreateInfo>(texture_data_infos[i])), img_result);
			const sl::size_t align = new_img.alignment();
			const sl::size_t offset = sl::aligned_to(total_size_out, align);
			total_size_out = offset + new_img.size_bytes();
			new (&ret[i]) image_allocation{offset, sl::move(new_img)};
		}
		return sl::move(ret);
	}

	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>
		asset_heap_allocation<I, Config, RenderProcessT>::
	initialize_image_views(
		sl::index_t alloc_idx
	) noexcept {
		_image_views[alloc_idx] = std::make_unique_for_overwrite<image_view[]>(_images[alloc_idx].size());
		for(sl::size_t i = 0; i < _images[alloc_idx].size(); ++i)
			RESULT_TRY_MOVE(_image_views[alloc_idx][i], make<image_view>(this->logi_device_ptr, _images[alloc_idx][i]));
		return {};
	}


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	bind(image_allocation&& img_alloc, sl::index_t alloc_idx) noexcept {
		_images[alloc_idx].push_back(sl::move(img_alloc.img));
		__D2D_VULKAN_VERIFY(vkBindImageMemory(*this->logi_device_ptr, _images[alloc_idx].back(), this->mems[alloc_idx], img_alloc.offset));
		return {};
	}


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	constexpr void    asset_heap_allocation<I, Config, RenderProcessT>::
	update_descriptors(sl::index_t alloc_idx) noexcept {
		sl::array<asset_usage_policy::num_usage_policies, std::vector<VkDescriptorImageInfo>> descriptor_infos{};
		for(sl::index_t i = 0; i < _images[alloc_idx].size(); ++i) {
			descriptor_infos[_image_usages[alloc_idx][i]].push_back(VkDescriptorImageInfo{
				.imageView{_image_views[alloc_idx][i]},
				.imageLayout{_images[alloc_idx][i].current_layout}
			});
		}
		for(sl::index_t i = 0; i < _samplers[alloc_idx].size(); ++i) {
			descriptor_infos[asset_usage_policy::sampler].push_back(VkDescriptorImageInfo{
				.sampler{_samplers[alloc_idx][i]}
			});
		}

		for(asset_usage_policy_t usage_idx = 0; usage_idx < asset_usage_policy::num_usage_policies; ++usage_idx) {
			const sl::size_t descriptor_count = descriptor_infos[usage_idx].size();
			if(descriptor_count == 0) continue;

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = _descriptor_sets[alloc_idx][usage_idx],
				.dstBinding = 0,
				.descriptorCount = static_cast<sl::uint32_t>(descriptor_count),
				.descriptorType = vk::descriptor_types[usage_idx],
				.pImageInfo = descriptor_infos[usage_idx].data(),
			};
			vkUpdateDescriptorSets(*this->logi_device_ptr, 1, &write, 0, nullptr);
		}
	}
}


namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
 	make_pools(
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts,
		sl::index_t alloc_idx
	) noexcept {
		{
		descriptor_pool_type tmp{this->logi_device_ptr};
		sl::array<asset_usage_policy::num_usage_policies, VkDescriptorPoolSize> pool_sizes{};
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i) {
			const sl::uint32_t asset_count = std::max(asset_counts[i], _descriptor_counts[alloc_idx][i]);
			pool_sizes[i] = {
				.type = vk::descriptor_types[i],
				.descriptorCount = std::max(asset_count, static_cast<sl::uint32_t>(1))
			};
		}
		VkDescriptorPoolCreateInfo pool_create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkDescriptorPoolCreateFlags{},
			.maxSets = asset_usage_policy::num_usage_policies,
			.poolSizeCount = pool_sizes.size(),
			.pPoolSizes = pool_sizes.data(),
		};
		__D2D_VULKAN_VERIFY(vkCreateDescriptorPool(*this->logi_device_ptr, &pool_create_info, nullptr, &tmp));

		_descriptor_pools[alloc_idx] = std::move(tmp);
		}

		{
		//Some graphics drivers are bugged and tweak out when you pass 0 as the asset count
		//So we make sure that there is always at least 1
		const sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> descriptor_counts = sl::make_deduced<sl::generic::array>(
			asset_counts,
			[](sl::uint32_t val, auto) {
				return std::max(val, static_cast<sl::uint32_t>(1));
			}
		);
		VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = asset_usage_policy::num_usage_policies,
			.pDescriptorCounts = descriptor_counts.data()
		};
		const sl::array<asset_usage_policy::num_usage_policies, VkDescriptorSetLayout> set_layout_handles =
			sl::make_deduced<sl::generic::array>(_descriptor_set_layouts, sl::functor::forward_construct<VkDescriptorSetLayout>{});
		const VkDescriptorSetAllocateInfo set_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variable_count_alloc_info,
			.descriptorPool = _descriptor_pools[alloc_idx],
			.descriptorSetCount = asset_usage_policy::num_usage_policies,
			.pSetLayouts = set_layout_handles.data()
		};

		sl::array<asset_usage_policy::num_usage_policies, VkDescriptorSet> set_handles;
		__D2D_VULKAN_VERIFY(vkAllocateDescriptorSets(*this->logi_device_ptr, &set_alloc_info, set_handles.data()));
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i)
			*(&_descriptor_sets[alloc_idx][i]) = set_handles[i];
		}

		return {};
	}


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<sl::index_t J, sl::size_t N, auto BufferIs>
	result<void>   asset_heap_allocation<I, Config, RenderProcessT>::
 	upload_image_data(
		buffer_segment<J, N, BufferIs> const& texture_data_buffer,
		sl::index_t alloc_idx,
		sl::index_t image_start_idx,
		sl::uint64_t timeout
	) noexcept {
		RenderProcessT& proc = static_cast<RenderProcessT&>(*this);
		const sl::index_t frame_idx = proc.frame_index();
		auto const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::image_data_upload];

		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, proc.begin_dedicated_copy(timeline::impl::dedicated_command_group::image_data_upload, timeout), pcwv_result);
		
		std::vector<texture_data_info> const& texture_data_infos = texture_data_buffer.texture_data_infos;
		for(sl::index_t i = 0; i < texture_data_infos.size(); ++i) {
			if(texture_data_infos[i].size == 0) continue;
			

			//VkBufferMemoryBarrier2 pre_copy_buffer_barrier{
			//    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//	.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			//	.srcAccessMask = VK_ACCESS_2_NONE,
			//	.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
			//	.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			//	.buffer = static_cast<VkBuffer>(texture_data_buffer),
			//	.offset = texture_data_infos[i].offset,
			//	.size = texture_data_infos[i].size
			//};
			VkImageMemoryBarrier2 pre_copy_image_barrier{
			    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			    .oldLayout = _images[alloc_idx][image_start_idx + i].layout(),
			    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    .image = _images[alloc_idx][image_start_idx + i],
			    .subresourceRange = {
			        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			        .baseMipLevel = 0,
			        .levelCount = _images[alloc_idx][image_start_idx + i].mip_level_count(),
			        .baseArrayLayer = 0,
			        .layerCount = _images[alloc_idx][image_start_idx + i].layer_count(),
			    },
			};
			transfer_command_buffer.pipeline_barrier({}, {/*&pre_copy_buffer_barrier, 1*/}, {&pre_copy_image_barrier, 1});

			const sl::uint32_t copy_region_count = std::min(_images[alloc_idx][image_start_idx + i].mip_level_count(), static_cast<sl::uint32_t>(max_mip_levels));
			std::unique_ptr<VkBufferImageCopy[]> copy_regions = std::make_unique_for_overwrite<VkBufferImageCopy[]>(copy_region_count);
			for(sl::uint32_t j = 0; j < copy_region_count; ++j) {
				new (&copy_regions[j]) VkBufferImageCopy{
					texture_data_infos[i].offset + texture_data_infos[i].mip_offsets[j],
					0, 0,
					VkImageSubresourceLayers{
					    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					    .mipLevel = j,
					    .baseArrayLayer = 0,
					    .layerCount = _images[alloc_idx][image_start_idx + i].layer_count(),
					},
					VkOffset3D{},
					VkExtent3D{
						.width = texture_data_infos[i].extent.width() >> j,
						.height = texture_data_infos[i].extent.height() >> j,
						.depth = 1
					}
				};
			}

			vkCmdCopyBufferToImage(transfer_command_buffer, 
				static_cast<VkBuffer>(texture_data_buffer),
				_images[alloc_idx][image_start_idx + i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				copy_region_count, copy_regions.get()
			);

			sl::array<1, VkImageMemoryBarrier2> post_copy_barriers{{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
				    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
				    .image = _images[alloc_idx][image_start_idx + i],
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = _images[alloc_idx][image_start_idx + i].mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = _images[alloc_idx][image_start_idx + i].layer_count(),
				    },
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, {}, post_copy_barriers);

			_images[alloc_idx][image_start_idx + i].current_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		}
		
		return proc.end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::image_data_upload, timeout);
	}
}

namespace acma::vk {


	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	template<sl::index_t J>
	result<void>    asset_heap_allocation<I, Config, RenderProcessT>::
	realloc(sl::index_constant_type<J>, sl::uint64_t timeout) noexcept
	requires(J == I) {
		if(desired_bytes == 0) return {};

		const sl::index_t alloc_idx = this->allocation_index();

		const memory_ptr_type old_mem = std::move(this->mems[alloc_idx]);
		std::vector<image> old_images = std::move(_images[alloc_idx]);
		std::vector<image>& images = _images[alloc_idx];
		
		//Re-initialize old memory
		this->mems[alloc_idx] = memory_ptr_type(this->logi_device_ptr);

		//Clone old images
		images.reserve(old_images.size());
		for(sl::index_t i = 0; i < old_images.size(); ++i) {
			RESULT_VERIFY_UNSCOPED(make<image>(this->logi_device_ptr, old_images[i].creation_info()), img_result);

			images.push_back(*sl::move(img_result));
		}

		//TODO do this once in physical_device
		//Get the memory properties
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*this->phys_device_ptr, &mem_props);

		//Find suitable memory for images
		{
		sl::uint32_t mem_type_idx = static_cast<uint32_t>(sl::npos);
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            for(std::size_t j = 0; j < images.size(); ++j) {
				const sl::uint32_t mem_type_bits = images[j].memory_requirements().memoryTypeBits;
                if(!(mem_type_bits & (1 << i)))
                    goto next_mem_prop;
			}
            if ((mem_props.memoryTypes[i].propertyFlags & ::acma::impl::flags_for<Config.image_memory>) == ::acma::impl::flags_for<Config.image_memory>) {
                mem_type_idx = i;
				break;
			}
        next_mem_prop:;
        }

		if(mem_type_idx == static_cast<std::uint32_t>(sl::npos)) [[unlikely]]
        	return errc::device_lacks_suitable_mem_type;

		VkMemoryAllocateInfo malloc_info{
		    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
		    .allocationSize = desired_bytes,
		    .memoryTypeIndex = mem_type_idx,
		};
		
		//Allocate new memory
		__D2D_VULKAN_VERIFY(vkAllocateMemory(*this->logi_device_ptr, &malloc_info, nullptr, &this->mems[alloc_idx]));
		}

		//Bind new images
		sl::uoffset_t offset = 0;
		for(std::size_t i = 0; i < images.size(); ++i) {
			offset = sl::aligned_to(offset, images[i].alignment());
			__D2D_VULKAN_VERIFY(vkBindImageMemory(*this->logi_device_ptr, images[i], this->mems[alloc_idx], offset))
			offset += images[i].size_bytes();
		}

		//Copy data from old images to new images
		RenderProcessT& proc = static_cast<RenderProcessT&>(*this);
		const sl::index_t frame_idx = proc.frame_index();
		auto const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::realloc];

		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, proc.begin_dedicated_copy(timeline::impl::dedicated_command_group::realloc, timeout), pcwv_result);

		for(sl::index_t j = 0; j < old_images.size(); ++j) {
			if(old_images[j].size_bytes() == 0) continue;
			
			const VkImageLayout original_layout = old_images[j].layout();

			sl::array<2, VkImageMemoryBarrier2> pre_copy_barriers {{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
					.srcAccessMask = VK_ACCESS_2_NONE,
					.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
				    .oldLayout = original_layout,
				    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				    .image = old_images[j],
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = old_images[j].mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = old_images[j].layer_count(),
				    },
				},
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
					.srcAccessMask = VK_ACCESS_2_NONE,
					.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				    .oldLayout = images[j].layout(),
				    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    .image = images[j],
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = images[j].mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = images[j].layer_count(),
				    },
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, {}, pre_copy_barriers);

			std::unique_ptr<VkImageCopy[]> copy_regions = std::make_unique_for_overwrite<VkImageCopy[]>(old_images[j].mip_level_count());
			for(sl::uint32_t i = 0; i < old_images[j].mip_level_count(); ++i) {
				new (&copy_regions[i]) VkImageCopy{
					VkImageSubresourceLayers{
					    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					    .mipLevel = i,
					    .baseArrayLayer = 0,
					    .layerCount = old_images[j].layer_count(),
					},
					VkOffset3D{},
					VkImageSubresourceLayers{
					    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					    .mipLevel = i,
					    .baseArrayLayer = 0,
					    .layerCount = images[j].layer_count(),
					},
					VkOffset3D{},
					old_images[j].size()
				};
			}

			vkCmdCopyImage(transfer_command_buffer, 
				old_images[j], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				images[j], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				old_images[j].mip_level_count(), copy_regions.get()
			);

			if(original_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
				images[j].current_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				continue;
			}

			sl::array<1, VkImageMemoryBarrier2> post_copy_barriers{{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
				    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    .newLayout = original_layout,
				    .image = images[j],
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = images[j].mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = images[j].layer_count(),
				    },
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, {}, post_copy_barriers);

			images[j].current_layout = original_layout;
		}

		return proc.end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::realloc, timeout);
	}
}