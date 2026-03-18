#pragma once
#include <vulkan/vulkan.h>

#include "sirius/core/asset_heap_config.hpp"
#include "sirius/graphics/core/texture_view.hpp"
#include "sirius/vulkan/core/command_buffer.fwd.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/core/buffer_config.hpp"
#include "sirius/vulkan/memory/generic_allocation.fwd.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"
//#include "sirius/core/render_process.fwd.hpp"
//#include "sirius/core/frames_in_flight.def.hpp"
//#include "sirius/vulkan/memory/asset_group.hpp"
//#include "sirius/vulkan/memory/asset_heap_allocation.hpp"
//#include "sirius/vulkan/memory/descriptor_heap.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkBuffer);


namespace acma::vk::impl {

	using buffer_ptr_type = vulkan_ptr<VkBuffer, vkDestroyBuffer>;
}


namespace acma::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N>, typename Derived>
    class device_allocation_segment_base;
}

namespace acma::vk {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N>, typename Derived>
    class device_allocation_segment;
}


namespace acma::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	struct device_allocation_segment_properties {
		constexpr static buffer_config config = sl::universal::get<sl::second_constant>(*std::next(BufferConfigs.begin(), I));
		constexpr static bool c = sl::universal::get<0>(sl::key_value_pair<int, int>{});
		constexpr static sl::size_t allocation_count = allocation_counts[config.coupling];

	protected:
		constexpr sl::index_t current_buffer_index() const noexcept;
	public:
		friend command_buffer;
	};
}

namespace acma::vk::impl {
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	class device_allocation_segment_base :
		public device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>
	{
		using base_type = device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
        static result<device_allocation_segment<I, N, BufferConfigs, RenderProcessT>> create(
			std::shared_ptr<logical_device> logi_device,
			std::size_t initial_capacity,
			std::size_t initial_size = 0
		) noexcept;

	public:
		constexpr std::byte const* data() const noexcept requires(memory_policy::is_cpu_visible(config.memory))  { return ptrs[this->current_buffer_index()]; }
		constexpr std::byte      * data()       noexcept requires(memory_policy::is_cpu_writable(config.memory)) { return ptrs[this->current_buffer_index()]; }

		constexpr sl::size_t size() const noexcept { return data_bytes; }
		constexpr sl::size_t size_bytes() const noexcept { return data_bytes; }
		constexpr sl::size_t capacity() const noexcept { return allocated_bytes; }
		constexpr sl::size_t capacity_bytes() const noexcept { return allocated_bytes; }

		constexpr gpu_address_t gpu_address() const noexcept { return device_addresses[this->current_buffer_index()]; }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(buffs[this->current_buffer_index()]); }
		constexpr explicit operator VkBuffer() const noexcept { return buffs[this->current_buffer_index()]; }
	
	public:
		constexpr result<void> reserve(sl::size_t new_capacity_bytes) noexcept;


	public:
		constexpr void clear() noexcept;

		constexpr result<void> resize(sl::size_t count_bytes) noexcept;
		constexpr result<void> try_resize(sl::size_t count_bytes) noexcept;

		// template<sl::size_t DstI, sl::size_t SrcI>
		// friend constexpr result<void> copy(
			// device_allocation_segment<DstI, N, BufferConfigs, RenderProcessT>& dst, 
			// device_allocation_segment<DstI, N, BufferConfigs, RenderProcessT> const& src, 
		// ) noexcept;

	public:
		friend ::acma::vk::command_buffer;

	protected:
		sl::array<allocation_count, buffer_ptr_type> buffs;
		sl::array<allocation_count, gpu_address_t> device_addresses;
		sl::array<allocation_count, std::byte*> ptrs;
        sl::size_t data_bytes;
        sl::size_t allocated_bytes;
		sl::size_t desired_bytes;
		sl::uoffset_t offset;
        VkBufferUsageFlags flags;
		VkDescriptorType descriptor_type;
	};


	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>::config.memory == memory_policy::push_constant
	)
	class device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT> :
		public device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>
	{
		using base_type = device_allocation_segment_properties<I, N, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;

	public:
		constexpr std::byte const* data() const noexcept { return bytes[this->current_buffer_index()].data(); }
		constexpr std::byte      * data()       noexcept { return bytes[this->current_buffer_index()].data(); }

		consteval sl::size_t size() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t size_bytes() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t capacity() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t capacity_bytes() const noexcept { return config.initial_capacity_bytes; }
		
	public:
		friend ::acma::vk::command_buffer;

	protected:
		sl::array<allocation_count, sl::array<config.initial_capacity_bytes, std::byte>> bytes;
	};
}


namespace acma::vk {
	//Directly modifyable
	//Assumes that reads/writes are done safely
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
    class device_allocation_segment<I, N, BufferConfigs, RenderProcessT> :
		public impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>
	{
	protected:
		using base_type = impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
		// static_assert(
		// 	!(device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		// 	device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
		// );

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


	protected:
		template<typename T>
		constexpr result<void> push_to(sl::uoffset_t offset, T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>);

		template<typename T, typename... Args>
		constexpr result<void> emplace_to(sl::uoffset_t dst_offset, Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>);
    };
}

namespace acma::vk {
	//Directly modifiable (specifically for textures)
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	class device_allocation_segment<I, N, BufferConfigs, RenderProcessT> : 
		public impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>
	{
	protected:
		using base_type = impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;

	public:
		constexpr result<void> push_back(texture_view t) noexcept;

		constexpr result<void> try_push_back(texture_view t) noexcept;

	private:
		template<sl::index_t, asset_heap_config, typename>
		friend class asset_heap_allocation;

	private:
		std::vector<texture_data_info> texture_data_infos;
	};
}

// namespace acma::vk {
// 	//Directly modifiable buffer for descriptor heaps
// 	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
// 	requires (
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		device_allocation_segment<I, N, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	class device_allocation_segment<I, N, BufferConfigs, RenderProcessT> : 
// 		public impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>
// 	{
// 		struct asset_group_info : descriptor_info {
// 			sl::size_t count;
// 		};
// 	private:
// 		template<sl::index_t J>
// 		using uniform_buffer_segment = device_allocation_segment<J, N, BufferConfigs, RenderProcessT>;
// 	protected:
// 		using base_type = impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>;
// 	public:
// 		using base_type::config;
// 		using base_type::allocation_count;
// 	public:
// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> emplace(
// 			asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);

// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> try_emplace(
// 			asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);


// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> emplace(
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);

// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> try_emplace(
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);
// 	private:
// 		template<bool ForceResize, asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> write_descriptors(
// 			sl::bool_constant_type<ForceResize>,
// 			asset_heap_allocation<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept;


// 	public:
// 		friend ::acma::vk::command_buffer;

// 	private:
// 		sl::array<descriptor_heap::num_descriptor_heaps, sl::size_t> heap_offsets;
// 	};
// }

namespace acma::vk {
	//Not directly modifyable
	template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	requires (
		impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT>::config.memory == memory_policy::gpu_local
	)
	class device_allocation_segment<I, N, BufferConfigs, RenderProcessT> : 
		public impl::device_allocation_segment_base<I, N, BufferConfigs, RenderProcessT> {};
}


#include "sirius/vulkan/memory/device_allocation_segment.inl"
