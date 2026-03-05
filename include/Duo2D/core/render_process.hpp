#pragma once
#include "Duo2D/core/render_process.fwd.hpp"

#include <vector>
#include <cstddef>
#include <memory>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "Duo2D/core/window.fwd.hpp"
#include "Duo2D/core/frames_in_flight.def.hpp"
#include "Duo2D/timeline/callback_event.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/memory/device_allocation_group.hpp"
#include "Duo2D/vulkan/memory/device_allocation_segment.hpp"
#include "Duo2D/vulkan/memory/device_allocation.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/display/framebuffer.hpp"
#include "Duo2D/vulkan/display/depth_image.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/core/asset_heap_key_t.hpp"



namespace d2d {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	class render_process :
		public vk::impl::device_allocation_group<
			BufferConfigs.size(), BufferConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>,
			sl::index_sequence_of_length_type<coupling_policy::num_coupling_policies>
		>
	{
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
			AssetHeapConfigs, sl::functor::subscript<0>{}, sl::functor::identity_index{}
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
		// template<asset_heap_key_t Key>
		// constexpr auto&& operator[](this auto&& self, sl::constant_type<asset_heap_key_t, Key>) noexcept 
		// requires (AssetHeapConfigs.contains(Key)) {
			// return static_cast<sl::copy_cvref_t<decltype(self), vk::device_allocation_segment<asset_heap_key_indices[Key], render_process>>>(self);
		// }
		
		// template<asset_heap_key_t Key>
		// constexpr auto&& get(this auto&& self, sl::constant_type<asset_heap_key_t, Key> = {}) noexcept 
		// requires (AssetHeapConfigs.contains(Key)) {
			// return sl::forward_like<decltype(self)>(self[sl::constant<asset_heap_key_t, Key>]);
		// }

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
		template<sl::size_t I, sl::size_t J>
		constexpr result<void> copy(
			allocation_segment_type<J> const& src,
			sl::size_t size,
			sl::uoffset_t offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept;



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

namespace d2d {
	template<sl::size_t I, sl::size_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
	constexpr result<void> copy(
		vk::device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst,
		vk::device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;
}



#include "Duo2D/core/render_process.inl"