#pragma once
#include "sirius/core/render_process.fwd.hpp"

#include <vector>
#include <memory>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/core/window.fwd.hpp"
#include "sirius/core/frames_in_flight.def.hpp"
#include "sirius/timeline/callback_event.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/memory/asset_heap_allocation_group.hpp"
#include "sirius/vulkan/memory/device_allocation_group.hpp"
#include "sirius/vulkan/memory/device_allocation_segment.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/core/asset_heap_key_t.hpp"



namespace acma {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	class render_process :
		public vk::impl::device_allocation_group<
			BufferConfigs.size(), BufferConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>,
			sl::index_sequence_of_length_type<coupling_policy::num_coupling_policies>
		>,
		public vk::impl::asset_heap_allocation_group<
			BufferConfigs.size(),
			AssetHeapConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>,
			sl::index_sequence_of_length_type<AssetHeapConfigs.size()>
		>
	{
		template<sl::index_t, sl::size_t _N, buffer_config_table<_N>, typename>
		friend class vk::impl::device_allocation_segment_base;
	private:
		using vk::impl::device_allocation_group<
			BufferConfigs.size(), BufferConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>,
			sl::index_sequence_of_length_type<coupling_policy::num_coupling_policies>
		>::realloc;
		using vk::impl::asset_heap_allocation_group<
			BufferConfigs.size(),
			AssetHeapConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>,
			sl::index_sequence_of_length_type<AssetHeapConfigs.size()>
		>::realloc;

	public:
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t command_buffer_count = CommandGroupCount;
		
	private:
		constexpr static sl::size_t N = BufferConfigs.size();
		constexpr static sl::size_t M = AssetHeapConfigs.size();

		constexpr static sl::lookup_table<N, buffer_key_t, sl::index_t> buffer_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			BufferConfigs, sl::functor::subscript<0>{}, sl::functor::identity_index{}
		);
		constexpr static sl::lookup_table<M, asset_heap_key_t, sl::index_t> asset_heap_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			AssetHeapConfigs, sl::functor::subscript<0>{}, []<sl::index_t I>(auto, sl::index_constant_type<I>){ return N + I; }
		);

		template<sl::index_t I>
		using allocation_segment_type = vk::device_allocation_segment<I, N, BufferConfigs, render_process>;
	public:
		using callback_function_type = result<void>(render_process&, window&, timeline::state&) noexcept;


	public:
		template<buffer_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<buffer_key_t, Key>) noexcept 
		requires (BufferConfigs.contains(Key)) {
			return static_cast<sl::copy_cvref_t<decltype(self), vk::device_allocation_segment<buffer_key_indices[Key], N, BufferConfigs, render_process>>>(self);
		}
		
		
		template<buffer_key_t Key>
		constexpr auto&& get(this auto&& self, sl::constant_type<buffer_key_t, Key> = {}) noexcept 
		requires (BufferConfigs.contains(Key)) {
			return sl::forward_like<decltype(self)>(self[sl::constant<buffer_key_t, Key>]);
		}
		
	public:
		template<asset_heap_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<asset_heap_key_t, Key>) noexcept 
		requires (AssetHeapConfigs.contains(Key)) {
			return static_cast<sl::copy_cvref_t<decltype(self), vk::asset_heap_allocation<asset_heap_key_indices[Key], AssetHeapConfigs[Key], render_process>>>(self);
		}
		
		template<asset_heap_key_t Key>
		constexpr auto&& get(this auto&& self, sl::constant_type<asset_heap_key_t, Key> = {}) noexcept 
		requires (AssetHeapConfigs.contains(Key)) {
			return sl::forward_like<decltype(self)>(self[sl::constant<asset_heap_key_t, Key>]);
		}

	public:
		constexpr std::shared_ptr<vk::logical_device>  logical_device_ptr()  const noexcept { return logi_device_ptr; }
		constexpr std::shared_ptr<vk::physical_device> physical_device_ptr() const noexcept { return phys_device_ptr; }

		constexpr sl::size_t  frame_count() const noexcept { return _frame_count; }
		constexpr sl::index_t frame_index() const noexcept { return frame_count() % frames_in_flight; }


		constexpr bool has_dedicated_present_queue() const noexcept {
			return 
				phys_device_ptr->queue_family_infos[command_family::graphics].index != 
				phys_device_ptr->queue_family_infos[command_family::present].index;
		}

		constexpr sl::array<command_family::num_families, std::shared_ptr<vk::command_pool>>       const& command_pool_ptrs (this auto const& self) noexcept { return self._command_pool_ptrs; }
		constexpr sl::array<frames_in_flight, sl::array<command_buffer_count, vk::command_buffer>> const& command_buffers   (this auto const& self) noexcept { return self._command_buffers; }
		
		constexpr sl::array<frames_in_flight, sl::array<command_family::num_families, vk::semaphore>> const& command_family_semaphores(this auto const& self) noexcept { return self._generic_timeline_sempahores; }
		constexpr sl::array<frames_in_flight, sl::array<command_buffer_count, vk::semaphore>>         const& command_buffer_semaphores(this auto const& self) noexcept { return self._command_buffer_semaphores; }
		constexpr std::vector<vk::semaphore>                                                          const& graphics_semaphores      (this auto const& self) noexcept { return self._graphics_semaphores; }
		constexpr std::vector<vk::semaphore>                                                          const& pre_present_semaphores   (this auto const& self) noexcept { return self._pre_present_semaphores; }
		constexpr std::array<vk::semaphore, frames_in_flight>                                         const& acquisition_semaphores   (this auto const& self) noexcept { return self._acquisition_semaphores; }


		constexpr auto&& command_family_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._command_family_semaphore_values); }
		constexpr auto&& command_buffer_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._command_buffer_semaphore_values); }

		constexpr auto&& timeline_callbacks(this auto&& self) noexcept {return sl::forward_like<decltype(self)>(self._timeline_callbacks); }


	public:
		//Any buffer to gpu_local buffer
		//(cpu_local_gpu_write buffer to gpu_local buffer not allowed?)
		template<sl::size_t DstI, sl::size_t SrcI>
		constexpr result<void> copy(
			allocation_segment_type<SrcI> const& src,
			sl::size_t size,
			sl::uoffset_t offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept
		requires(
			!memory_policy::is_cpu_visible(allocation_segment_type<DstI>::config.memory)
		);

		//Any host-visible buffer to any other host-visible buffer
		//(Any host-visible buffer to cpu_local_gpu_write buffer not allowed)
		template<sl::size_t DstI, sl::size_t SrcI>
		constexpr result<void> copy(
			allocation_segment_type<SrcI> const& src,
			sl::size_t size,
			sl::uoffset_t offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept
		requires(
			memory_policy::is_cpu_visible(allocation_segment_type<DstI>::config.memory) &&
			memory_policy::is_cpu_visible(allocation_segment_type<SrcI>::config.memory)
		);


		//gpu_local buffer to cpu_local_gpu_write buffer
		//(gpu_local buffer to cpu_local_upload buffer not allowed)

	public:
		constexpr result<sl::uint64_t> begin_dedicated_copy(sl::index_t command_group_idx, sl::uint64_t timeout) & noexcept;
		constexpr result<void> end_dedicated_copy(sl::uint64_t wait_value, sl::index_t command_group_idx, sl::uint64_t timeout) const& noexcept;


	protected:
		std::shared_ptr<vk::logical_device> logi_device_ptr;
		std::shared_ptr<vk::physical_device> phys_device_ptr;

		//std::size_t frame_idx;
		sl::array<timeline::callback_event::num_callback_events, std::vector<callback_function_type*>> _timeline_callbacks;

		sl::array<command_family::num_families, std::shared_ptr<vk::command_pool>> _command_pool_ptrs;
		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::command_buffer>> _command_buffers;
		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::semaphore>> _command_buffer_semaphores;
		sl::array<frames_in_flight, sl::array<command_buffer_count, sl::uint64_t>> _command_buffer_semaphore_values;
		sl::array<frames_in_flight, sl::array<command_family::num_families, vk::semaphore>> _generic_timeline_sempahores;
		sl::array<frames_in_flight, sl::array<command_family::num_families, sl::uint64_t>> _command_family_semaphore_values;
		std::array<vk::semaphore, frames_in_flight> _acquisition_semaphores;
		std::vector<vk::semaphore> _graphics_semaphores;
		std::vector<vk::semaphore> _pre_present_semaphores;

		sl::size_t _frame_count;
	};
}

namespace acma {
	template<sl::size_t DstI, sl::size_t SrcI, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void> copy(
		vk::device_allocation_segment<DstI, N, BufferConfigs, RenderProcessT>& dst,
		vk::device_allocation_segment<SrcI, N, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;
}



#include "sirius/core/render_process.inl"