#pragma once
#include "Duo2D/vulkan/memory/device_allocation_segment.hpp"
#include <streamline/functional/functor/address_of.hpp>

#include <vulkan/vulkan.h>

#include "Duo2D/core/error.hpp"


namespace d2d::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    result<device_allocation_segment<I, N, BufferConfigs, RenderProcessT>> device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::create(std::shared_ptr<logical_device> device, std::size_t initial_capacity, std::size_t initial_size) noexcept {
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
		

		ret.flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;


		for(sl::index_t i = 0; i < ret.buffs.size(); ++i) {
			ret.buffs[i] = buffer_ptr_type{device};
			VkBufferCreateInfo buffer_create_info{
    		    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            	.size = initial_capacity,
    		    .usage = ret.flags,
    		    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    		};

    		__D2D_VULKAN_VERIFY(vkCreateBuffer(*device, &buffer_create_info, nullptr, &ret.buffs[i]));
		}
        return ret;
    }
}



namespace d2d::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    constexpr sl::index_t device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>::current_buffer_index() const noexcept {
		return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
	}
}



namespace d2d::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::
	reserve(sl::size_t new_capacity_bytes) noexcept {
		if(new_capacity_bytes <= this->capacity_bytes()) 
			return {};
		
		this->desired_bytes = new_capacity_bytes;
		RESULT_VERIFY((static_cast<RenderProcessT&>(*this).realloc(sl::index_constant<I>)));
		return {};
	}
}


namespace d2d::vk {
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


namespace d2d::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
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

namespace d2d::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
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

namespace d2d::vk {
    template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
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