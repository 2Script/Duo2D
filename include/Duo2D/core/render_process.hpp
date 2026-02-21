#pragma once
#include "Duo2D/core/render_process.fwd.hpp"

#include <vector>
#include <cstddef>
#include <memory>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "Duo2D/core/frames_in_flight.def.hpp"
#include "Duo2D/timeline/callbacks.hpp"
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
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"




namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t BP, memory_policy_t MP>
	using device_allocation_filter_sequence = sl::filtered_sequence_t<
		sl::index_sequence_of_length_type<N>,
		[]<sl::index_t I>(sl::index_constant_type<I>){
			return 
				vk::device_allocation_segment<I, ::d2d::render_process<N, Resources>>::config.memory == MP &&
				vk::device_allocation_segment<I, ::d2d::render_process<N, Resources>>::config.buffering == BP;
		}
	>;
}


namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t BufferingPolicy, memory_policy_t... MemoryPolicyIs>
	class device_allocation_group<N, Resources, BufferingPolicy, sl::index_sequence_type<MemoryPolicyIs...>> : 
		public vk::device_allocation<
			D2D_FRAMES_IN_FLIGHT, 
			device_allocation_filter_sequence<N, Resources, BufferingPolicy, MemoryPolicyIs>,
			BufferingPolicy, MemoryPolicyIs,
			::d2d::render_process<N, Resources>
		>... 
	{};
}


namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t... BufferingPolicyIs>
	class render_process<N, Resources, sl::index_sequence_type<BufferingPolicyIs...>> : 
		public device_allocation_group<
			N, Resources, BufferingPolicyIs, sl::index_sequence_of_length_type<memory_policy::num_memory_policies>
		>... 
	{
		template<buffering_policy_t BP, memory_policy_t MP>
		using allocation_type = vk::device_allocation<
			D2D_FRAMES_IN_FLIGHT,
			device_allocation_filter_sequence<N, Resources, BP, MP>,
			BP, MP,
			::d2d::render_process<N, Resources>
		>;
	private:
		template<sl::index_t, typename>
		friend class vk::device_allocation_segment;
		template<sl::index_t, typename>
		friend class vk::impl::device_allocation_segment_base;
	protected:
		template<buffering_policy_t BP, memory_policy_t MP>
		using memory_type = vk::device_allocation<
			D2D_FRAMES_IN_FLIGHT, 
			device_allocation_filter_sequence<N, Resources, BP, MP>,
			BP, MP,
			::d2d::render_process<N, Resources>
		>;
	protected:
		template<memory_policy_t... MPs>
		result<void> initialize_allocations(sl::integer_sequence_type<memory_policy_t, MPs...>) noexcept;
	public:
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t command_buffer_count = frames_in_flight;// + 1;
		constexpr static sl::lookup_table<N, resource_key_t, sl::index_t> resource_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			Resources, sl::functor::subscript<0>{}, sl::functor::identity_index{}
		);
	
	public:
		result<bool> verify_swap_chain(VkResult fn_result, bool even_if_suboptimal) noexcept;

	public:
		using timeline_callbacks_type = timeline::callbacks<N, Resources>;

	public:    
		constexpr std::shared_ptr<vk::logical_device>  logical_device_ptr()  const noexcept { return logi_device_ptr; }
		constexpr std::shared_ptr<vk::physical_device> physical_device_ptr() const noexcept { return phys_device_ptr; }

		constexpr sl::array<command_family::num_families, std::shared_ptr<vk::command_pool>>                     const& command_pool_ptrs()         const& noexcept { return _command_pool_ptrs; }
		constexpr std::array<std::array<vk::command_buffer<N>, command_family::num_families>, frames_in_flight>     const& command_buffers()           const& noexcept { return _command_buffers; }
		constexpr std::array<std::array<vk::semaphore, command_family::num_families>, frames_in_flight>          const& command_buffer_semaphores() const& noexcept { return _command_buffer_semaphores; }
		constexpr std::array<vk::fence, frames_in_flight>                                                        const& graphics_fences()           const& noexcept { return _graphics_fences; }
		constexpr std::array<vk::semaphore, command_family::num_families>                                        const& generic_semaphores()        const& noexcept { return _generic_timeline_sempahores; }
		constexpr std::vector<vk::semaphore>                                                                     const& graphics_semaphores()       const& noexcept { return _graphics_semaphores; }
		constexpr std::vector<vk::semaphore>                                                                     const& pre_present_semaphores()    const& noexcept { return _pre_present_semaphores; }
		constexpr std::array<vk::semaphore, frames_in_flight>                                                    const& acquisition_semaphores()    const& noexcept { return _acquisition_semaphores; }
		constexpr vk::surface                                                                                    const& surface()                   const& noexcept { return _surface; }
		constexpr vk::swap_chain                                                                                 const& swap_chain()                const& noexcept { return _swap_chain; }
		constexpr vk::depth_image                                                                                const& depth_image()               const& noexcept { return _depth_image; }
		constexpr std::vector<timeline_callbacks_type>                                                           const& timeline_callbacks()        const& noexcept { return _timeline_callbacks; }
		

		constexpr auto&& generic_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._generic_semaphore_values); } 
		constexpr auto&& command_buffer_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._command_buffer_semaphore_values); } 


		constexpr sl::size_t  frame_count()                 const noexcept { return _frame_count; }
		constexpr sl::index_t frame_index()                 const noexcept { return frame_count() % frames_in_flight; }
		//consteval sl::index_t outside_of_rendering_index()  const noexcept { return frames_in_flight; }
		constexpr extent2     screen_size()                 const noexcept { return _size; }
		constexpr bool        has_dedicated_present_queue() const noexcept {
			return phys_device_ptr->queue_family_infos[command_family::graphics].index != phys_device_ptr->queue_family_infos[command_family::present].index;
		}

	public:
		template<resource_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<resource_key_t, Key>) noexcept 
		requires (Resources.contains(Key)) {
			return static_cast<sl::copy_cvref_t<decltype(self), vk::device_allocation_segment<resource_key_indices[Key], render_process>>>(self);
		}
		
		template<resource_key_t Key>
		constexpr auto&& get(this auto&& self) noexcept 
		requires (Resources.contains(Key)) {
			return sl::forward_like<decltype(self)>(self[sl::constant<resource_key_t, Key>]);
		}
		
		
		template<sl::size_t I, sl::size_t J>
		constexpr result<void> copy(
			vk::device_allocation_segment<J, render_process> const& src,
			sl::size_t size,
			sl::uoffset_t offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept;
	protected:
		constexpr static std::array<vk::pixel_format_info, 2> pixel_format_priority = {vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_SRGB)->second, vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_UNORM)->second};
		constexpr static vk::color_space_info default_color_space = vk::color_spaces.find(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)->second;
		constexpr static std::array<vk::present_mode, static_cast<std::size_t>(vk::present_mode::num_present_modes)> present_mode_priority{vk::present_mode::mailbox, vk::present_mode::fifo, vk::present_mode::fifo_relaxed, vk::present_mode::immediate}; 
	   

	protected:
        std::unique_ptr<GLFWwindow, sl::functor::generic_stateless<glfwDestroyWindow>> window_handle;

		std::shared_ptr<vk::logical_device> logi_device_ptr;
		std::shared_ptr<vk::physical_device> phys_device_ptr;

		vk::surface _surface;
		vk::swap_chain _swap_chain;
		vk::depth_image _depth_image;
		extent2 _size;

		//std::size_t frame_idx;

		std::vector<timeline_callbacks_type> _timeline_callbacks;
		sl::array<command_family::num_families, std::shared_ptr<vk::command_pool>> _command_pool_ptrs;
		std::array<std::array<vk::command_buffer<N>, command_family::num_families>, frames_in_flight> _command_buffers;
		std::array<std::array<vk::semaphore, command_family::num_families>, frames_in_flight> _command_buffer_semaphores;
		std::array<std::array<std::unique_ptr<std::atomic<sl::uint64_t>>, command_family::num_families>, frames_in_flight> _command_buffer_semaphore_values;
		std::array<vk::fence, frames_in_flight> _graphics_fences;
		std::array<vk::semaphore, frames_in_flight> _acquisition_semaphores;
		std::vector<vk::semaphore> _graphics_semaphores;
		std::vector<vk::semaphore> _pre_present_semaphores;
		std::array<vk::semaphore, command_family::num_families> _generic_timeline_sempahores;
		sl::array<command_family::num_families, sl::uint64_t> _generic_semaphore_values;

		sl::size_t _frame_count;
	};
}

namespace d2d {
	template<sl::size_t I, sl::size_t J, sl::size_t N, resource_table<N> Resources>
	constexpr result<void> copy(
		vk::device_allocation_segment<I, render_process<N, Resources>>& dst,
		vk::device_allocation_segment<J, render_process<N, Resources>> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;
}



#include "Duo2D/core/render_process.inl"