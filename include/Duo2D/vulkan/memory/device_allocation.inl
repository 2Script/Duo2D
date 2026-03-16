#pragma once
#include "Duo2D/vulkan/memory/device_allocation.hpp"

#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/default_construct.hpp>
#include <streamline/functional/functor/invoke_each_result.hpp>

#include "Duo2D/timeline/dedicated_command_group.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"


namespace d2d::vk {
    template<sl::size_t FiF, sl::index_t... Is, coupling_policy_t CP, memory_policy_t MP, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    result<device_allocation<FiF, sl::index_sequence_type<Is...>, CP, MP, N, BufferConfigs, RenderProcessT>>
		device_allocation<FiF, sl::index_sequence_type<Is...>, CP, MP, N, BufferConfigs, RenderProcessT>::
	create(
		std::shared_ptr<logical_device> logi_device,
		std::shared_ptr<physical_device> phys_device//,
		//std::shared_ptr<command_pool> transfer_command_pool
	) noexcept {
        device_allocation ret{}; 
        RESULT_VERIFY(ret.initialize(logi_device, phys_device));//, transfer_command_pool));

		constexpr auto make_single_buffer = []<sl::index_t I>(
			device_allocation& ret,
			sl::index_constant_type<I>
		) noexcept -> result<void> {
			return ret.make_buffer(sl::index_constant<I>);
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, make_single_buffer>{}(sl::index_sequence<Is...>, ret)));
		
		constexpr static sl::array<allocation_count, sl::index_t> alloc_indices = sl::universal::make_deduced<sl::generic::array>(sl::index_sequence_of_length<allocation_count>);
		RESULT_VERIFY(ret.initialize_buffers(alloc_indices));
        return ret;
    }
}


namespace d2d::vk {
    template<sl::size_t FiF, sl::index_t... Is, coupling_policy_t CP, memory_policy_t MP, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	template<sl::index_t I>
    constexpr result<void>
		device_allocation<FiF, sl::index_sequence_type<Is...>, CP, MP, N, BufferConfigs, RenderProcessT>::
	make_buffer(sl::index_constant_type<I>) noexcept {
		constexpr std::size_t buff_capacity_bytes = std::max(
			segment_type<I>::config.initial_capacity_bytes,
			minimum_buffer_capacity_size_bytes
		);

        RESULT_TRY_MOVE(static_cast<segment_type<I>&>(*this), (make<segment_type<I>>(this->logi_device_ptr, buff_capacity_bytes, 0)));
		segment_type<I>::allocated_bytes = buff_capacity_bytes;
		return {};
	}


    template<sl::size_t FiF, sl::index_t... Is, coupling_policy_t CP, memory_policy_t MP, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	template<sl::size_t AllocIdxCount>
    result<void>
		device_allocation<FiF, sl::index_sequence_type<Is...>, CP, MP, N, BufferConfigs, RenderProcessT>::
	initialize_buffers(sl::array<AllocIdxCount, sl::index_t> alloc_indices) noexcept {
		//Get the memory requirements for each buffer
		//We assume that duplicate buffers have the same memory requirements, so we only check the first buffer
		using mem_reqs_table = sl::lookup_table<buffer_count, sl::index_t, VkMemoryRequirements>;
		constinit static mem_reqs_table mem_reqs = sl::universal::make<mem_reqs_table>(
			sl::index_sequence<Is...>, sl::functor::identity{}, sl::functor::default_construct<VkMemoryRequirements>{}
		);
		(vkGetBufferMemoryRequirements(*this->logi_device_ptr, static_cast<VkBuffer>(segment_type<Is>::buffs[0]), &mem_reqs[Is]), ...);
		
		//TODO do this once in physical_device
		//Get the memory properties
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*this->phys_device_ptr, &mem_props);

		//Find the index of a suitable GPU memory type
		sl::uint32_t mem_type_idx = static_cast<uint32_t>(sl::npos);
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            for(std::size_t j = 0; j < buffer_count; ++j) {
				const sl::uint32_t mem_type_bits = std::next(mem_reqs.begin(), j)->value.memoryTypeBits;
                if(!(mem_type_bits & (1 << i)))
                    goto next_mem_prop;
			}
            if ((mem_props.memoryTypes[i].propertyFlags & ::d2d::impl::flags_for<MP>) == ::d2d::impl::flags_for<MP>) {
                mem_type_idx = i;
				break;
			}
        next_mem_prop:;
        }
        
		if(mem_type_idx == static_cast<std::uint32_t>(sl::npos)) [[unlikely]]
        	return errc::device_lacks_suitable_mem_type;


		//Calulate total memory size and the memory offset for each buffer
		constexpr auto calc_offset = []<sl::index_t I>(sl::size_t& alloc_size, sl::index_constant_type<I>) noexcept -> sl::uoffset_t {
			const sl::uoffset_t offset = sl::aligned_to(alloc_size, mem_reqs[I].alignment);
			alloc_size = offset + mem_reqs[I].size;
			return offset;
		};
		sl::size_t allocation_size = 0;
		const sl::lookup_table<buffer_count, sl::index_t, sl::uoffset_t> segment_offsets{{{
			{Is, (calc_offset(allocation_size, sl::index_constant<Is>))}...
		}}};
		
		// const sl::lookup_table<buffer_count, sl::index_t, sl::size_t> segment_sizes = sl::universal::make_deduced<sl::generic::lookup_table>(
		// 	sl::index_sequence<Is...>, sl::functor::identity{}, [segment_offsets]<sl::index_t J>(auto, sl::index_constant_type<J>){
		// 		if constexpr (J == buffer_count - 1)
		// 			return sl::universal::get<sl::second_constant>(*std::next(mem_reqs.begin(), J)).size;
		// 		else return 
		// 			sl::universal::get<sl::second_constant>(*std::next(segment_offsets.begin(), J + 1)) -
		// 			sl::universal::get<sl::second_constant>(*std::next(segment_offsets.begin(), J));
		// 	}
		// );
		
		(([this, segment_offsets/*, segment_sizes*/](){
			segment_type<Is>::offset = segment_offsets[Is];
			//segment_type<Is>::allocated_bytes = segment_sizes[Is];
		}()), ...);


		//Allocate the memory
		if(allocation_size == 0) return {};
		
		VkMemoryAllocateFlagsInfo malloc_flags_info {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
		};
		VkMemoryAllocateInfo malloc_info{
		    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &malloc_flags_info,
		    .allocationSize = allocation_size,
		    .memoryTypeIndex = mem_type_idx,
		};
		
		for(std::size_t i = 0; i < alloc_indices.size(); ++i)
			__D2D_VULKAN_VERIFY(vkAllocateMemory(*this->logi_device_ptr, &malloc_info, nullptr, &this->mems[alloc_indices[i]]));

			
		//Bind all the buffers to the raw memory and get their gpu address
		RESULT_VERIFY(ol::to_result((([this, alloc_indices]() noexcept -> result<void> {
			for(std::size_t i = 0; i < alloc_indices.size(); ++i) {
				__D2D_VULKAN_VERIFY(vkBindBufferMemory(
					*this->logi_device_ptr, 
					segment_type<Is>::buffs[alloc_indices[i]],
					this->mems[alloc_indices[i]], 
					segment_type<Is>::offset
				));
				VkBufferDeviceAddressInfo device_address_info{
					.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
					.buffer = segment_type<Is>::buffs[alloc_indices[i]]
				};
				segment_type<Is>::device_addresses[alloc_indices[i]] = vkGetBufferDeviceAddress(*this->logi_device_ptr, &device_address_info);
			}
			return {};
		}) && ...)))


		//Map host-visible buffers
		if constexpr(memory_policy::is_cpu_visible(MP)) {
			for(std::size_t i = 0; i < alloc_indices.size(); ++i) {
				void* map;
				__D2D_VULKAN_VERIFY(vkMapMemory(*this->logi_device_ptr, this->mems[alloc_indices[i]], 0, VK_WHOLE_SIZE, 0, &map));
				std::byte* base_ptr = std::launder(reinterpret_cast<std::byte*>(map));
				((segment_type<Is>::ptrs[alloc_indices[i]] = base_ptr + segment_type<Is>::offset), ...);
			}
		}

        return {};
	}
}

namespace d2d::vk{
    template<sl::size_t FiF, sl::index_t... Is, coupling_policy_t CP, memory_policy_t MP, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
		template<sl::index_t I>
    result<void>    device_allocation<FiF, sl::index_sequence_type<Is...>, CP, MP, N, BufferConfigs, RenderProcessT>::
	realloc(sl::index_constant_type<I>, sl::uint64_t timeout) noexcept 
	requires((I == Is) || ...) {
		const sl::index_t i = this->allocation_index();

		const sl::array<buffer_count, sl::size_t> buff_sizes{{
			segment_type<Is>::size_bytes()...
		}};

		const memory_ptr_type old_mem = std::move(this->mems[i]);
		const sl::array<buffer_count, impl::buffer_ptr_type> old_buffs{{
			std::move(segment_type<Is>::buffs[i])...
		}};
		const sl::array<buffer_count, std::byte*> old_ptrs{{
			segment_type<Is>::ptrs[i]...
		}};
		
		//Re-initialize old memory
		this->mems[i] = memory_ptr_type(this->logi_device_ptr);

		//Clone old buffers
		RESULT_VERIFY(ol::to_result((([this, i]() noexcept -> result<void> {
			segment_type<Is>::buffs[i] = impl::buffer_ptr_type{this->logi_device_ptr};
			const sl::size_t buffer_allocation_size = std::max(segment_type<Is>::desired_bytes, segment_type<Is>::allocated_bytes);
			VkBufferCreateInfo buffer_create_info{
			    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			    .size = buffer_allocation_size,
			    .usage = segment_type<Is>::flags,
			    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};
			__D2D_VULKAN_VERIFY(vkCreateBuffer(*this->logi_device_ptr, &buffer_create_info, nullptr, &segment_type<Is>::buffs[i]));
			segment_type<Is>::allocated_bytes = buffer_allocation_size;
			return {};
		}) && ...)));

		//Initialize as usual
		RESULT_VERIFY(initialize_buffers(sl::array<1, sl::index_t>{{i}}));

		//If all old buffers were empty, then we're done
		constexpr static sl::array<buffer_count, sl::size_t> all_zeros{};
		if(std::memcmp(buff_sizes.data(), all_zeros.data(), buffer_count * sizeof(sl::size_t)) == 0)
			return {};


		//Copy data from old buffers to new buffers
		
		//For host-writable buffers, just do a memcpy
		if constexpr(memory_policy::is_cpu_writable(MP)) {
			const sl::array<buffer_count, std::byte*> new_ptrs{{
				segment_type<Is>::ptrs[i]...
			}};
			
			for(sl::index_t j = 0; j < buffer_count; ++j)
				std::memcpy(new_ptrs[j], old_ptrs[j], buff_sizes[j]);
			
			return {};
		}


		const sl::array<buffer_count, impl::buffer_ptr_type*> buff_refs{{
			std::addressof(segment_type<Is>::buffs[i])...
		}};
		
		RenderProcessT& proc = static_cast<RenderProcessT&>(*this);
		const sl::index_t frame_idx = proc.frame_index();
		vk::command_buffer const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::realloc];
		
		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, proc.begin_dedicated_copy(timeline::impl::dedicated_command_group::realloc, timeout), pcwv_result);

		for(sl::index_t j = 0; j < buffer_count; ++j) {
			if(buff_sizes[j] == 0) continue;
			
			VkBufferCopy copy_region{
            	.srcOffset = 0,
            	.dstOffset = 0,
            	.size = buff_sizes[j],
			};
        	vkCmdCopyBuffer(transfer_command_buffer, old_buffs[j], *buff_refs[j], 1, &copy_region);

			sl::array<2, VkBufferMemoryBarrier2> post_copy_barriers {{
				VkBufferMemoryBarrier2{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.buffer = old_buffs[j],
					.offset = 0,
					.size = buff_sizes[j]
				},
				VkBufferMemoryBarrier2{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.buffer = *buff_refs[j],
					.offset = 0,
					.size = buff_sizes[j]
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, post_copy_barriers, {});
		}
		
		return proc.end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::realloc, timeout);
	}
}
