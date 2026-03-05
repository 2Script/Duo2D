#pragma once
#include "Duo2D/vulkan/memory/device_allocation.fwd.hpp"

#include <vulkan/vulkan.h>
#include <streamline/containers/array.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "Duo2D/core/render_process.fwd.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/memory/device_allocation_segment.hpp"
#include "Duo2D/core/memory_policy.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"


namespace d2d::vk::impl {
	struct memory_requirements : VkMemoryRequirements {
		memory_policy_t policy;
	};

	template<sl::size_t N>
	struct allocation_info {
		sl::array<N, sl::size_t> buffer_offsets;
		sl::size_t allocation_size; 
	};
}

namespace d2d::vk {
    template<sl::size_t FramesInFlight, sl::index_t... Is, coupling_policy_t CouplingPolicy, memory_policy_t MemoryPolicy, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
    class device_allocation<FramesInFlight, sl::index_sequence_type<Is...>, CouplingPolicy, MemoryPolicy, N, BufferConfigs, RenderProcessT> : 
		public device_allocation_segment<Is, N, BufferConfigs, RenderProcessT>...
	{
	private:
		constexpr static sl::size_t minimum_buffer_capacity_size_bytes = sizeof(std::byte) * 16;
	private:
		constexpr static sl::size_t allocation_count = impl::allocation_counts[CouplingPolicy];
		constexpr static sl::size_t buffer_count = sizeof...(Is); 
		constexpr static sl::array<N, buffer_key_t> buffer_keys = sl::universal::make_deduced<sl::generic::array>(BufferConfigs, sl::functor::subscript<0>{});
	public:
		template<sl::index_t I>
		using segment_type = device_allocation_segment<I, N, BufferConfigs, RenderProcessT>;
		using memory_ptr_type = vulkan_ptr<VkDeviceMemory, vkFreeMemory>;
	private:
		constexpr static bool is_host_visible_memory(memory_policy_t policy) noexcept { 
			return policy != memory_policy::gpu_local; 
		}
	private:
		constexpr sl::index_t allocation_index() noexcept {
			return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
		}

	// public:
	// 	template<buffer_key_t K>
    // 	constexpr auto&& operator[](this auto&& self, buffer_key_constant_type<K>) noexcept { 
	// 		return static_cast<sl::copy_cvref_t<decltype(self), device_allocation_segment<MemoryPolicy, K, device_allocation<MemoryPolicy, BufferConfigs, sl::index_sequence_type<MatchingIs...>>>>>(self);
	// 	}

	public:
		static result<device_allocation> create(
			std::shared_ptr<logical_device> logi_device, 
			std::shared_ptr<physical_device> phys_device,
			std::shared_ptr<command_pool> transfer_command_pool
		) noexcept;
    private:
		template<sl::size_t AllocIdxCount>
		result<void> initialize_buffers(sl::array<AllocIdxCount, sl::index_t> alloc_indices) noexcept;

	protected:
		template<sl::index_t I>
		result<void> realloc(sl::index_constant_type<I>, sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires((I == Is) || ...);

	private:
		sl::array<allocation_count, memory_ptr_type> mems;
		sl::array<allocation_count, vk::command_buffer> realloc_command_buffers;
		sl::array<allocation_count, vk::fence> realloc_fences;
		
	private:
		std::shared_ptr<logical_device> logi_device_ptr;
		std::shared_ptr<physical_device> phys_device_ptr;
    };
}

namespace d2d::vk {
    template<sl::size_t FramesInFlight, sl::index_t... Is, coupling_policy_t CouplingPolicy, memory_policy_t MemoryPolicy, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires (sizeof...(Is) == 0 || MemoryPolicy == memory_policy::push_constant)
    class device_allocation<FramesInFlight, sl::index_sequence_type<Is...>, CouplingPolicy, MemoryPolicy, N, BufferConfigs, RenderProcessT> : 
		public device_allocation_segment<Is, N, BufferConfigs, RenderProcessT>...
	{
	public:
		template<sl::index_t I>
		using segment_type = device_allocation_segment<I, N, BufferConfigs, RenderProcessT>;
		using memory_ptr_type = vulkan_ptr<VkDeviceMemory, vkFreeMemory>;
	public:
		static result<device_allocation> create(std::shared_ptr<logical_device>, std::shared_ptr<physical_device>, std::shared_ptr<command_pool>) noexcept {
			return device_allocation{};
		}
		
		template<sl::index_t I>
		result<void> realloc(sl::index_constant_type<I>, sl::uint64_t = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires((I == Is) || ...) { return {}; }
	};
}

#include "Duo2D/vulkan/memory/device_allocation.inl"
