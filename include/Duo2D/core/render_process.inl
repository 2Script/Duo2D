#pragma once
#include "Duo2D/core/render_process.hpp"


namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t... BufferingPolicyIs>
	template<memory_policy_t... MPs>
	result<void>    render_process<N, Resources, sl::index_sequence_type<BufferingPolicyIs...>>::
	initialize_allocations(sl::integer_sequence_type<memory_policy_t, MPs...>) noexcept {
		auto init_single_alloc = [this]<buffering_policy_t BP>(sl::constant_type<buffering_policy_t, BP>) -> result<void> {
			return ol::to_result((([this]() -> result<void> {
				RESULT_TRY_MOVE(
					(static_cast<allocation_type<BP, MPs>&>(*this)),
					(make<allocation_type<BP, MPs>>(logi_device_ptr, phys_device_ptr))
				);
				return {};
			}) && ...));
		};

		return ol::to_result((([&init_single_alloc]() -> result<void> {
			return init_single_alloc(sl::constant<buffering_policy_t, BufferingPolicyIs>);
		}) && ...));
	}
}


namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t... BufferingPolicyIs>
	result<bool>    render_process<N, Resources, sl::index_sequence_type<BufferingPolicyIs...>>::
	verify_swap_chain(VkResult fn_result, bool even_if_suboptimal) noexcept {
		switch(fn_result) {
		case VK_SUCCESS:
			return false;
		case VK_SUBOPTIMAL_KHR:
			if(!even_if_suboptimal) return false;
			[[fallthrough]];
		case VK_ERROR_OUT_OF_DATE_KHR: {
			if(glfwWindowShouldClose(this->window_handle.get())) [[unlikely]] return false;
			vkDeviceWaitIdle(*this->logi_device_ptr);
			//VkFormat old_format = _swap_chain.format().pixel_format.id;
			_swap_chain = {}; //delete before remaking swapchain
			RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(this->logi_device_ptr, this->phys_device_ptr, pixel_format_priority, default_color_space, present_mode_priority, _surface, this->window_handle.get()));
			//if(old_format != _swap_chain.format().pixel_format.id) {
			//	RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(this->logi_device_ptr, _swap_chain.format()));
			//	//RESULT_VERIFY(this->create_descriptors(_render_pass));
			//}

			RESULT_TRY_MOVE(_depth_image, make<vk::depth_image>(this->logi_device_ptr, this->phys_device_ptr, _swap_chain.extent()));
			//for(std::size_t i = 0; i < _swap_chain.image_count(); ++i)
			//	RESULT_TRY_MOVE(_framebuffers[i], make<vk::framebuffer>(this->logi_device_ptr, _swap_chain.image_views()[i], _depth_image.view(), _render_pass, _swap_chain.extent()));
			//(Ts::on_swap_chain_update(*this, this->template uniform_map<Ts>()), ...);
			return true;
		}
		default: 
			return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(fn_result));
		}
	}
}

namespace d2d::impl {
	template<sl::size_t N, resource_table<N> Resources, buffering_policy_t... BufferingPolicyIs>
	template<sl::size_t I, sl::size_t J>
	constexpr result<void>    render_process<N, Resources, sl::index_sequence_type<BufferingPolicyIs...>>::
	copy(
		vk::device_allocation_segment<J, render_process> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) & noexcept {
		vk::device_allocation_segment<I, render_process>& dst = static_cast<vk::device_allocation_segment<I, render_process>&>(*this);
		//TODO: use next frame index if theres no garauntee current transfer command buffer is not in use
		vk::command_buffer<N> const& transfer_command_buffer = _command_buffers[frame_index()][command_family::transfer];
		const sl::uint64_t semaphore_value = command_buffer_semaphore_values()[frame_index()][command_family::transfer]->fetch_add(1, std::memory_order::relaxed);

		VkSemaphoreWaitInfo semaphore_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &command_buffer_semaphores()[frame_index()][command_family::transfer],
			.pValues = &semaphore_value
		};
		vk::semaphore_submit_info semaphore_signal_info{
			command_buffer_semaphores()[frame_index()][command_family::transfer],
			render_stage::group::all_transfer,
			semaphore_value + 1,
		};

		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*logi_device_ptr, &semaphore_wait_info, std::numeric_limits<sl::uint64_t>::max()));
		RESULT_VERIFY(transfer_command_buffer.reset());
        RESULT_VERIFY(transfer_command_buffer.begin(true));
		transfer_command_buffer.copy(dst, src, size, dst_offset, src_offset);
		RESULT_VERIFY(transfer_command_buffer.end());
		RESULT_VERIFY(transfer_command_buffer.submit(command_family::transfer, {}, {&semaphore_signal_info, 1}));
		return {};
	}
}

namespace d2d {
	template<sl::size_t I, sl::size_t J, sl::size_t N, resource_table<N> Resources>
	constexpr result<void> copy(
		vk::device_allocation_segment<I, render_process<N, Resources>>& dst,
		vk::device_allocation_segment<J, render_process<N, Resources>> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) noexcept {
		render_process<N, Resources>& proc = static_cast<render_process<N, Resources>&>(dst);
		return proc.template copy<I>(src, size, dst_offset, src_offset);
	}
}

namespace d2d{
	
}