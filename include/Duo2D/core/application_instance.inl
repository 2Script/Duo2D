#pragma once
#include "Duo2D/core/application_instance.hpp"

#include <cstring>
#include <memory>
#include <string_view>
#include <utility>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/default_construct.hpp>
#include <streamline/functional/functor/invoke_each_result.hpp>
#include <streamline/functional/functor/forward_construct.hpp>

#include <GLFW/glfw3.h>
#include <result/verify.h>
#include <vulkan/vulkan_core.h>


#include "Duo2D/timeline/command.fwd.hpp"
#include "Duo2D/core/invoke_all.def.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/core/thread_pool.hpp"
#include "Duo2D/input/codes_map.hpp"
#include "Duo2D/input/window_info.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/input/combination.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"


namespace d2d {
	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<bool WindowCapability>
	result<application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>>     application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::
	create(
		std::shared_ptr<vk::instance> instance,
		std::shared_ptr<vk::logical_device> logi_device, 
		std::shared_ptr<vk::physical_device> phys_device,
		std::string_view name,
		sl::bool_constant_type<WindowCapability>
	) noexcept {
		application_instance ret{};
		ret.inst_ptr = instance;
		ret.phys_device_ptr = phys_device;
		ret.logi_device_ptr = logi_device;
		ret.app_name = name;

		constexpr sl::size_t command_familes_to_init = command_family::num_distinct_families + static_cast<sl::size_t>(WindowCapability);

		//Create command pools
		for(command_family_t i = 0; i < command_familes_to_init; ++i) {
			RESULT_VERIFY_UNSCOPED((make<vk::command_pool>(i, logi_device, phys_device)), c);
			ret._command_pool_ptrs[i] = std::make_shared<vk::command_pool>(*std::move(c));
		}

		
		//Initialize allocations
		constexpr auto init_single_alloc = []<coupling_policy_t CP, memory_policy_t MP>(
			application_instance& app_inst,
			sl::constant_type<coupling_policy_t, CP>,
			sl::constant_type<memory_policy_t, MP>
		) noexcept -> result<void> {
			using allocation_type = vk::device_allocation<
				frames_in_flight, 
				vk::impl::device_allocation_filter_sequence<N, BufferConfigs, render_process_type, CP, MP>,
				CP, MP,
				N, BufferConfigs,
				render_process_type
			>;
			RESULT_TRY_MOVE(
				(static_cast<allocation_type&>(app_inst)),
				(make<allocation_type>(app_inst.logi_device_ptr, app_inst.phys_device_ptr, app_inst._command_pool_ptrs[command_family::transfer]))
			);
			return {};
		};
		
		RESULT_VERIFY((sl::functor::invoke_each_result<
			result<void>, 
			sl::functor::invoke_each_result<result<void>, init_single_alloc>{}
		>{}(
			sl::integer_sequence_of_length<coupling_policy_t, coupling_policy::num_coupling_policies>,
			sl::integer_sequence_of_length<memory_policy_t, memory_policy::num_memory_policies>,
			ret
		)));


		constexpr static sl::size_t dedicated_cmd_buff_count = timeline::impl::dedicated_command_group::num_dedicated_command_groups;
		for(sl::index_t i = 0; i < ret.frames_in_flight; ++i) {
			//Create command buffers
			for(sl::index_t j = 0; j < dedicated_cmd_buff_count; ++j) {
				RESULT_TRY_MOVE(ret._command_buffers[i][j], make<vk::command_buffer>(
					logi_device, 
					phys_device, 
					ret._command_pool_ptrs[command_family::transfer]
				));
			}
			for(sl::index_t j = 0; j < command_traits_type::group_count; ++j) {
				if(command_traits_type::group_families[j] == command_family::none) continue;
				RESULT_TRY_MOVE(ret._command_buffers[i][j + dedicated_cmd_buff_count], make<vk::command_buffer>(
					logi_device, 
					phys_device, 
					ret._command_pool_ptrs[command_traits_type::group_families[j]]
				));
			}
			
			//Create command buffer semaphores
			for(sl::index_t j = 0; j < command_group_count; ++j) {
				RESULT_TRY_MOVE(ret._command_buffer_semaphores[i][j], make<vk::semaphore>(logi_device, VK_SEMAPHORE_TYPE_TIMELINE));
				ret._command_buffer_semaphore_values[i][j] = 0;
			}


			//Create generic semaphores
			for(sl::index_t j = 0; j < command_familes_to_init; ++j) {
				RESULT_TRY_MOVE(ret._generic_timeline_sempahores[i][j], make<vk::semaphore>(logi_device, VK_SEMAPHORE_TYPE_TIMELINE));
				ret._command_family_semaphore_values[i][j] = 0;
			}

			//Create image acquire semaphore
			RESULT_TRY_MOVE(ret._acquisition_semaphores[i], make<vk::semaphore>(logi_device));
		}

		return ret;
	}


	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	result<void>     application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::
	emplace_window(d2d::sz2u32 size, std::string_view title) noexcept {
		//Create window
		RESULT_TRY_MOVE(static_cast<window&>(*this), make<window>(
			inst_ptr, 
			size,
			title.empty() ? app_name : title
		));
		RESULT_VERIFY(window::initialize(this->logi_device_ptr, this->phys_device_ptr));
		
		//Create swap chain sempahores
		this->_graphics_semaphores.reserve(this->_swap_chain.image_count());
		this->_pre_present_semaphores.reserve(this->_swap_chain.image_count());
		for(std::size_t i = 0; i < this->_swap_chain.image_count(); ++i) {
			RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(this->logi_device_ptr), graphics_semaphore);
			this->_graphics_semaphores.push_back(*std::move(graphics_semaphore));

			RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(this->logi_device_ptr), pre_present_semaphore);
			this->_pre_present_semaphores.push_back(*std::move(pre_present_semaphore));
		}


		D2D_INVOKE_ALL(this->timeline_callbacks(), on_swap_chain_updated, *this, *this, external_timeline_state());
		return {};
	}


	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	result<void>     application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::
	initialize() noexcept {
		//Create auxiliary objects
		constexpr auto init_aux = []<sl::index_t I>(application_instance& app_inst, window& win, sl::index_constant_type<I>) noexcept -> result<void> {
			if constexpr (std::is_same_v<typename sl::tuple_traits<decltype(app_inst.auxiliary)>::template type_of_element<I>, sl::empty_t>) {
				//_auxiliary[sl::index_constant<I>] = sl::empty_t{};
				return {};
			}
			RESULT_TRY_MOVE(app_inst.auxiliary[sl::index_constant<I>], (sl::type_of_pack_element_t<I, timeline::setup<TimelineEventTs>...>{}(app_inst, win)));
			return {};
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, init_aux>{}(sl::index_sequence_for_pack<TimelineEventTs...>, *this, *this)));
		
			
		return {};
	}
}


namespace d2d {
	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    result<void> application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::render() noexcept {
		using filter_dedicated_command_groups_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<command_group_count>,
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept { return I >= d2d::timeline::impl::dedicated_command_group::num_dedicated_command_groups; }
		>;
        //wait for rendering to finish last frame
		const sl::index_t frame_idx = this->frame_index();
		const sl::array<command_traits_type::group_count, VkSemaphore> wait_semaphores = sl::universal::make_deduced<sl::generic::array>(
			this->_command_buffer_semaphores[frame_idx], 
			sl::functor::forward_construct<VkSemaphore>{},
			filter_dedicated_command_groups_sequence{}
		);
		const sl::array<command_traits_type::group_count, sl::uint64_t> wait_semaphores_values = sl::universal::make_deduced<sl::generic::array>(
			this->_command_buffer_semaphore_values[frame_idx], 
			sl::functor::forward_construct<sl::uint64_t>{},
			filter_dedicated_command_groups_sequence{}
		);

		VkSemaphoreWaitInfo wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = command_traits_type::group_count,
			.pSemaphores = wait_semaphores.data(),
			.pValues = wait_semaphores_values.data(),
		};
		__D2D_VULKAN_VERIFY(vkWaitSemaphores(*this->logical_device_ptr(), &wait_info, std::numeric_limits<std::uint64_t>::max()));

		timeline::state timeline_state{
			.image_index = 0
		};
		
		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_begin, *this, *this, timeline_state);

		
		constexpr auto exec = []<sl::index_t I>(application_instance& app_inst, timeline_state_type& state, sl::index_constant_type<I>) noexcept -> result<void> {
			return app_inst.template execute_command<I>(state);
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, exec>{}(sl::index_sequence_for_pack<TimelineEventTs...>, *this, timeline_state)));
        //((Ts{}(*this, timeline_state)), ...);


		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_end, *this, *this, timeline_state);

        //frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        ++this->_frame_count;
		//this->frame_count.fetch_add();
        return {};
    }


	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<sl::index_t I>
    result<void> application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::execute_command(timeline_state_type& state) noexcept {
		using timeline_type = typename sl::tuple_traits<sl::tuple<TimelineEventTs...>>::template type_of_element<I>;
		return d2d::timeline::command<timeline_type>{}(
			*this,
			*this,
			state,
			sl::universal::get<I>(auxiliary), 
			sl::index_constant<command_traits_type::group_indices[I] + timeline::impl::dedicated_command_group::num_dedicated_command_groups>
		);
	}
}

namespace d2d {
	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<typename TimelineCommandT>
    result<void> application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::execute_command(timeline_state_type& state) noexcept {
		return d2d::timeline::command<TimelineCommandT>{}(
			*this,
			*this,
			state,
			sl::empty_t{},
			timeline::impl::dedicated_command_group::out_of_timeline_execute
		);
	}

	template<typename... TimelineEventTs, auto BufferConfigs, auto AssetHeapConfigs> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<typename TimelineCommandT>
    result<void> application_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, AssetHeapConfigs>::execute_command() noexcept {
		return execute_command(external_timeline_state());
	}
}
