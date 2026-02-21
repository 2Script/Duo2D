#pragma once
#include <vulkan/vulkan.h>

#include "Duo2D/core/render_process.fwd.hpp"
#include "Duo2D/vulkan/core/command_buffer.fwd.hpp"
#include "Duo2D/core/frames_in_flight.def.hpp"
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/core/resource_config.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);


namespace d2d::vk::impl {
	constexpr sl::array<buffering_policy::num_buffering_policies, sl::size_t> allocation_counts{{1, D2D_FRAMES_IN_FLIGHT}};

	using buffer_ptr_type = vulkan_ptr<VkBuffer, vkDestroyBuffer>;
}


namespace d2d::vk::impl {
	template<sl::index_t I, typename Derived>
    class device_allocation_segment_base;
}

namespace d2d::vk {
	template<sl::index_t I, typename Derived>
    class device_allocation_segment;
}


namespace d2d::vk::impl {
	template<sl::index_t I, sl::size_t N, resource_table<N> Resources>
	struct device_allocation_segment_properties {
		constexpr static resource_config config = sl::universal::get<sl::second_constant>(*std::next(Resources.begin(), I));
		constexpr static bool c = sl::universal::get<0>(sl::key_value_pair<int, int>{});
		constexpr static sl::size_t allocation_count = allocation_counts[config.buffering];

	protected:
		constexpr sl::index_t current_buffer_index() const noexcept;
	public:
		friend command_buffer<N>;
	};
}

namespace d2d::vk::impl {
	template<sl::index_t I, sl::size_t N, resource_table<N> Resources>
	class device_allocation_segment_base<I, render_process<N, Resources>> :
		public device_allocation_segment_properties<I, N, Resources>
	{
		using base_type = device_allocation_segment_properties<I, N, Resources>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
        static result<device_allocation_segment<I, render_process<N, Resources>>> create(std::shared_ptr<logical_device> device, std::size_t initial_capacity, std::size_t initial_size = 0) noexcept;

	public:
		constexpr std::byte const* data() const noexcept { return ptrs[this->current_buffer_index()]; }
		constexpr std::byte      * data()       noexcept { return ptrs[this->current_buffer_index()]; }

		constexpr sl::size_t size() const noexcept { return data_bytes; }
		constexpr sl::size_t size_bytes() const noexcept { return data_bytes; }
		constexpr sl::size_t capacity() const noexcept { return allocated_bytes; }
		constexpr sl::size_t capacity_bytes() const noexcept { return allocated_bytes; }

		constexpr VkDeviceAddress gpu_address() const noexcept { return device_addresses[this->current_buffer_index()]; }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(buffs[this->current_buffer_index()]); }
	
	public:
		constexpr result<void> reserve(sl::size_t new_capacity_bytes) noexcept;


	public:
		constexpr void clear() noexcept;

		constexpr result<void> resize(sl::size_t count_bytes) noexcept;
		constexpr result<void> try_resize(sl::size_t count_bytes) noexcept;

		// template<sl::size_t DstI, sl::size_t SrcI>
		// friend constexpr result<void> copy(
			// device_allocation_segment<DstI, render_process<N, Resources>>& dst, 
			// device_allocation_segment<DstI, render_process<N, Resources>> const& src, 
		// ) noexcept;

	public:
		friend ::d2d::vk::command_buffer<N>;

	protected:
		sl::array<allocation_count, buffer_ptr_type> buffs;
		sl::array<allocation_count, VkDeviceAddress> device_addresses;
		sl::array<allocation_count, std::byte*> ptrs;
        sl::size_t data_bytes;
        sl::size_t allocated_bytes;
		sl::size_t desired_bytes;
		sl::uoffset_t offset;
        VkBufferUsageFlags flags;
		VkDescriptorType descriptor_type;
	};


	template<sl::index_t I, sl::size_t N, resource_table<N> Resources>
	requires(
		device_allocation_segment_properties<I, N, Resources>::config.memory == memory_policy::push_constant
	)
	class device_allocation_segment_base<I, render_process<N, Resources>> :
		public device_allocation_segment_properties<I, N, Resources>
	{
		using base_type = device_allocation_segment_properties<I, N, Resources>;
	public:
		using base_type::config;
		using base_type::allocation_count;

	public:
		constexpr std::byte const* data() const noexcept { return bytes[this->current_buffer_index()].data(); }
		constexpr std::byte      * data()       noexcept { return bytes[this->current_buffer_index()].data(); }

		consteval sl::size_t size() const noexcept { return bytes.size(); }
		consteval sl::size_t size_bytes() const noexcept { return bytes.size(); }
		consteval sl::size_t capacity() const noexcept { return bytes.size(); }
		consteval sl::size_t capacity_bytes() const noexcept { return bytes.size(); }
		
	public:
		friend ::d2d::vk::command_buffer<N>;

	protected:
		sl::array<allocation_count, sl::array<config.initial_capacity_bytes, std::byte>> bytes;
	};
}


namespace d2d::vk {
	//Directly modifyable
	//Assumes that reads/writes are done safely
	template<sl::index_t I, sl::size_t N, resource_table<N> Resources>
    class device_allocation_segment<I, render_process<N, Resources>> :
		public impl::device_allocation_segment_base<I, render_process<N, Resources>>
	{
	protected:
		using base_type = impl::device_allocation_segment_base<I, render_process<N, Resources>>;
	public:
		using base_type::config;
		using base_type::allocation_count;

	public:
		template<typename T>
		constexpr result<void> push_back(T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
		requires(sl::traits::is_constructible_from_v<T, T&&> && config.memory != memory_policy::push_constant);

		template<typename T>
		constexpr result<void> try_push_back(T&& t)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
		requires(sl::traits::is_constructible_from_v<T, T&&>);


		template<typename T, typename... Args>
		constexpr result<void> emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...> && config.memory != memory_policy::push_constant);

		template<typename T, typename... Args>
		constexpr result<void> try_emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...>);


	private:
		template<typename T>
		constexpr result<void> push_to(sl::uoffset_t offset, T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>);

		template<typename T, typename... Args>
		constexpr result<void> emplace_to(sl::uoffset_t dst_offset, Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>);
    };
}

namespace d2d::vk {
	//Not directly modifyable
	template<sl::index_t I, sl::size_t N, resource_table<N> Resources>
	requires (
		device_allocation_segment<I, render_process<N, Resources>>::config.memory == memory_policy::gpu_local
	)
	class device_allocation_segment<I, render_process<N, Resources>> : 
		public impl::device_allocation_segment_base<I, render_process<N, Resources>> {};
}


#include "Duo2D/vulkan/memory/device_allocation_segment.inl"
