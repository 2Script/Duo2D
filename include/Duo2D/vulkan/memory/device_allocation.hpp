#pragma once
#include "Duo2D/vulkan/memory/device_allocation.fwd.hpp"

#include <vulkan/vulkan.h>
#include <streamline/containers/array.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "Duo2D/core/render_process.fwd.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/vulkan/memory/generic_allocation.hpp"
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
		public generic_allocation<CouplingPolicy, RenderProcessT>,
		public device_allocation_segment<Is, N, BufferConfigs, RenderProcessT>...
	{
		using base_type = generic_allocation<CouplingPolicy, RenderProcessT>;
	private:
		constexpr static sl::size_t minimum_buffer_capacity_size_bytes = sizeof(std::byte) * 16;
	private:
		constexpr static sl::size_t buffer_count = sizeof...(Is); 
		constexpr static sl::array<N, buffer_key_t> buffer_keys = sl::universal::make_deduced<sl::generic::array>(BufferConfigs, sl::functor::subscript<0>{});
	public:
		using base_type::allocation_count;
	public:
		template<sl::index_t I>
		using segment_type = device_allocation_segment<I, N, BufferConfigs, RenderProcessT>;
		using typename base_type::memory_ptr_type;
	private:
		constexpr sl::index_t allocation_index() noexcept {
			return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
		}

	public:
		static result<device_allocation> create(
			std::shared_ptr<logical_device> logi_device, 
			std::shared_ptr<physical_device> phys_device//,
			//std::shared_ptr<command_pool> transfer_command_pool
		) noexcept;
    private:
		template<sl::index_t I>
		constexpr result<void> make_buffer(sl::index_constant_type<I>) noexcept;
		template<sl::size_t AllocIdxCount>
		result<void> initialize_buffers(sl::array<AllocIdxCount, sl::index_t> alloc_indices) noexcept;

	protected:
		template<sl::index_t I>
		result<void> realloc(sl::index_constant_type<I>, sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires((I == Is) || ...);
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
	public:
		template<typename... Args>
		static result<device_allocation> create(Args&&...) noexcept {
			return device_allocation{};
		}
		
		template<sl::index_t I>
		result<void> realloc(sl::index_constant_type<I>, sl::uint64_t = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires((I == Is) || ...) { return {}; }
	};
}

#include "Duo2D/vulkan/memory/device_allocation.inl"
