#pragma once
#include "Duo2D/vulkan/memory/device_allocation.hpp"

#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/default_construct.hpp>

#include "Duo2D/timeline/dedicated_command_group.hpp"


namespace d2d::vk {
    template<sl::size_t FiF, sl::index_t... Is, buffering_policy_t BP, memory_policy_t MP, sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
    result<device_allocation<FiF, sl::index_sequence_type<Is...>, BP, MP, render_process<N, Resources, CommandGroupCount>>>
		device_allocation<FiF, sl::index_sequence_type<Is...>, BP, MP, render_process<N, Resources, CommandGroupCount>>::
	create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device) noexcept {
        device_allocation ret{}; 
        ret.mems = sl::universal::make<sl::array<allocation_count, memory_ptr_type>>(
			logi_device, 
			sl::in_place_repeat_tag<allocation_count>, 
			[](std::shared_ptr<logical_device>& logi_device_ptr, auto) noexcept {
				return memory_ptr_type(logi_device_ptr);
			}
		);
		ret.logi_device_ptr = logi_device;
		ret.phys_device_ptr = phys_device;

		RESULT_VERIFY(ol::to_result((([&logi_device, &ret]() noexcept -> result<void> {
			const std::size_t buff_capacity_bytes = std::max(
				segment_type<Is>::config.initial_capacity_bytes,
				minimum_buffer_capacity_size_bytes
			);

			segment_type<Is>& segment = static_cast<segment_type<Is>&>(ret);
            RESULT_TRY_MOVE(segment, (make<segment_type<Is>>(logi_device, buff_capacity_bytes, 0)));
			return {};
		}) && ...)));
		constexpr static sl::array<allocation_count, sl::index_t> alloc_indices = sl::universal::make_deduced<sl::generic::array>(sl::index_sequence_of_length<allocation_count>);
		RESULT_VERIFY(ret.initialize_buffers(alloc_indices));
        return ret;
    }
}


namespace d2d::vk {
    template<sl::size_t FiF, sl::index_t... Is, buffering_policy_t BP, memory_policy_t MP, sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
	template<sl::size_t AllocIdxCount>
    result<void>
		device_allocation<FiF, sl::index_sequence_type<Is...>, BP, MP, render_process<N, Resources, CommandGroupCount>>::
	initialize_buffers(sl::array<AllocIdxCount, sl::index_t> alloc_indices) noexcept {
		//Get the memory requirements for each buffer
		//We assume that duplicate buffers have the same memory requirements, so we only check the first buffer
		using mem_reqs_table = sl::lookup_table<buffer_count, sl::index_t, VkMemoryRequirements>;
		constinit static mem_reqs_table mem_reqs = sl::universal::make<mem_reqs_table>(
			sl::index_sequence<Is...>, sl::functor::identity{}, sl::functor::default_construct<VkMemoryRequirements>{}
		);
		(vkGetBufferMemoryRequirements(*logi_device_ptr, static_cast<VkBuffer>(segment_type<Is>::buffs[0]), &mem_reqs[Is]), ...);
		
		//TODO do this once in physical_device
		//Get the memory properties
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(*phys_device_ptr, &mem_props);

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
		
		const sl::lookup_table<buffer_count, sl::index_t, sl::size_t> segment_sizes = sl::universal::make_deduced<sl::generic::lookup_table>(
			sl::index_sequence<Is...>, sl::functor::identity{}, [segment_offsets]<sl::index_t J>(auto, sl::index_constant_type<J>){
				if constexpr (J == buffer_count - 1)
					return sl::universal::get<sl::second_constant>(*std::next(mem_reqs.begin(), J)).size;
				else return 
					sl::universal::get<sl::second_constant>(*std::next(segment_offsets.begin(), J + 1)) -
					sl::universal::get<sl::second_constant>(*std::next(segment_offsets.begin(), J));
			}
		);
		
		(([this, segment_offsets, segment_sizes](){
			segment_type<Is>::offset = segment_offsets[Is];
			segment_type<Is>::allocated_bytes = segment_sizes[Is];
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
			__D2D_VULKAN_VERIFY(vkAllocateMemory(*logi_device_ptr, &malloc_info, nullptr, &mems[alloc_indices[i]]));

			
		//Bind all the buffers to the raw memory and get their gpu address
		RESULT_VERIFY(ol::to_result((([this, alloc_indices]() noexcept -> result<void> {
			for(std::size_t i = 0; i < alloc_indices.size(); ++i) {
				__D2D_VULKAN_VERIFY(vkBindBufferMemory(
					*logi_device_ptr, 
					segment_type<Is>::buffs[alloc_indices[i]],
					mems[alloc_indices[i]], 
					segment_type<Is>::offset
				));
				VkBufferDeviceAddressInfo device_address_info{
					.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
					.buffer = segment_type<Is>::buffs[alloc_indices[i]]
				};
				segment_type<Is>::device_addresses[alloc_indices[i]] = vkGetBufferDeviceAddress(*logi_device_ptr, &device_address_info);
			}
			return {};
		}) && ...)))


		//Map host-visible buffers
		if constexpr(is_host_visible_memory(MP)) {
			for(std::size_t i = 0; i < alloc_indices.size(); ++i) {
				void* map;
				__D2D_VULKAN_VERIFY(vkMapMemory(*logi_device_ptr, mems[alloc_indices[i]], 0, VK_WHOLE_SIZE, 0, &map));
				std::byte* base_ptr = std::launder(reinterpret_cast<std::byte*>(map));
				((segment_type<Is>::ptrs[alloc_indices[i]] = base_ptr + segment_type<Is>::offset), ...);
			}
		}

        return {};
	}
}

namespace d2d::vk{
    template<sl::size_t FiF, sl::index_t... Is, buffering_policy_t BP, memory_policy_t MP, sl::size_t N, resource_table<N> Resources, sl::size_t CommandGroupCount>
    result<void>    device_allocation<FiF, sl::index_sequence_type<Is...>, BP, MP, render_process<N, Resources, CommandGroupCount>>::
	realloc(sl::uint64_t timeout) noexcept {
		const sl::index_t i = allocation_index();

		const sl::array<buffer_count, sl::size_t> buff_sizes{{
			segment_type<Is>::size_bytes()...
		}};

		const memory_ptr_type old_mem = std::move(mems[i]);
		const sl::array<buffer_count, impl::buffer_ptr_type> old_buffs{{
			std::move(segment_type<Is>::buffs[i])...
		}};
		
		//Re-initialize old memory
		mems[i] = memory_ptr_type(logi_device_ptr);

		//Clone old buffers
		RESULT_VERIFY(ol::to_result((([this, i]() noexcept -> result<void> {
			segment_type<Is>::buffs[i] = impl::buffer_ptr_type{logi_device_ptr};
			VkBufferCreateInfo buffer_create_info{
			    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			    .size = segment_type<Is>::desired_bytes,
			    .usage = segment_type<Is>::flags,
			    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};
			__D2D_VULKAN_VERIFY(vkCreateBuffer(*logi_device_ptr, &buffer_create_info, nullptr, &segment_type<Is>::buffs[i]));
			return {};
		}) && ...)));

		//Initialize as usual
		RESULT_VERIFY(initialize_buffers(sl::array<1, sl::index_t>{{i}}));

		//If all old buffers were empty, then we're done
		constexpr static sl::array<buffer_count, sl::size_t> all_zeros{};
		if(std::memcmp(buff_sizes.data(), all_zeros.data(), buffer_count * sizeof(sl::size_t)) == 0)
			return {};


		//Copy data from old buffers to new buffers
		const sl::array<buffer_count, impl::buffer_ptr_type*> buff_refs{{
			std::addressof(segment_type<Is>::buffs[i])...
		}};

		render_process<N, Resources, CommandGroupCount>& proc = static_cast<render_process<N, Resources, CommandGroupCount>&>(*this);
		const sl::index_t frame_idx = proc.frame_index();
		vk::command_buffer<N> const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::realloc];

		const sl::uint64_t semaphore_pre_value = proc.command_buffer_semaphore_values()[frame_idx][timeline::impl::dedicated_command_group::realloc]++;
		const sl::uint64_t semaphore_post_value = semaphore_pre_value + 1;

		VkSemaphoreWaitInfo semaphore_pre_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &proc.command_buffer_semaphores()[frame_idx][timeline::impl::dedicated_command_group::realloc],
			.pValues = &semaphore_pre_value
		};
		vk::semaphore_submit_info semaphore_signal_info{
			proc.command_buffer_semaphores()[frame_idx][timeline::impl::dedicated_command_group::realloc],
			render_stage::group::all_transfer,
			semaphore_post_value,
		};
		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*logi_device_ptr, &semaphore_pre_wait_info, timeout));
		
		RESULT_VERIFY(transfer_command_buffer.reset());
        RESULT_VERIFY(transfer_command_buffer.begin(true));
		for(sl::index_t j = 0; j < buffer_count; ++j) {
			if(buff_sizes[j] == 0) continue;
			VkBufferCopy copy_region{
            	.srcOffset = 0,
            	.dstOffset = 0,
            	.size = buff_sizes[j],
			};
        	vkCmdCopyBuffer(transfer_command_buffer, old_buffs[j], *buff_refs[j], 1, &copy_region);
		}
		RESULT_VERIFY(transfer_command_buffer.end());
		RESULT_VERIFY(transfer_command_buffer.submit(command_family::transfer, {}, {&semaphore_signal_info, 1}));


		VkSemaphoreWaitInfo semaphore_post_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &proc.command_buffer_semaphores()[frame_idx][timeline::impl::dedicated_command_group::realloc],
			.pValues = &semaphore_post_value
		};
		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*logi_device_ptr, &semaphore_post_wait_info, timeout));

		return {};
	}
}
