#pragma once
#include "Duo2D/core/window.hpp"

#include <cstring>
#include <memory>
#include <string_view>
#include <utility>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/default_construct.hpp>
#include <streamline/functional/functor/invoke_each_result.hpp>

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
    template<typename... Ts, auto Resources>
	requires impl::is_resource_table_v<decltype(Resources)>
    result<window<sl::tuple<Ts...>, Resources>>    window<sl::tuple<Ts...>, Resources>::
	create(std::string_view title, unsigned int width, unsigned int height, std::shared_ptr<vk::instance> i) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        window ret{};
		ret.window_handle = std::unique_ptr<GLFWwindow, sl::functor::generic_stateless<glfwDestroyWindow>>(glfwCreateWindow(width, height, title.data(), nullptr, nullptr));
        __D2D_GLFW_VERIFY(ret.window_handle);

        //Set callbacks
        glfwSetKeyCallback(ret.window_handle.get(), kb_key_input);
        glfwSetCharCallback(ret.window_handle.get(), kb_text_input);
        glfwSetCursorPosCallback(ret.window_handle.get(), mouse_move);
        glfwSetMouseButtonCallback(ret.window_handle.get(), mouse_button_input);
        glfwSetScrollCallback(ret.window_handle.get(), mouse_scroll);

        //Create surface
        RESULT_TRY_MOVE(ret._surface, make<vk::surface>(ret.window_handle.get(), i));

        //Verify window size
        int w = 0, h = 0;
        glfwGetWindowSize(ret.window_handle.get(), &w, &h);
        __D2D_GLFW_VERIFY((width == static_cast<unsigned int>(w) && height == static_cast<unsigned int>(h)));
        ret._size = {width, height};

        return std::move(ret);
    }


    template<typename... Ts, auto Resources> 
	requires impl::is_resource_table_v<decltype(Resources)>
	result<void>     window<sl::tuple<Ts...>, Resources>::
	initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device) noexcept {
		if(this->_swap_chain) 
			return {}; //return error::window_already_initialized;

		this->phys_device_ptr = phys_device;
		this->logi_device_ptr = logi_device;

		RESULT_VERIFY(this->initialize_allocations(sl::integer_sequence_of_length<memory_policy_t, memory_policy::num_memory_policies>));

		//Create command pools
		for(command_family_t i = 0; i < command_family::num_families; ++i) {
			RESULT_VERIFY_UNSCOPED((make<vk::command_pool>(i, logi_device, phys_device)), c);
			this->_command_pool_ptrs[i] = std::make_shared<vk::command_pool>(*std::move(c));
		}


		//Create swap chain
		RESULT_TRY_MOVE(this->_swap_chain, make<vk::swap_chain>(
			logi_device, 
			phys_device,
			render_process<Resources.size(), Resources>::pixel_format_priority,
			render_process<Resources.size(), Resources>::default_color_space,
			render_process<Resources.size(), Resources>::present_mode_priority,
			this->_surface,
			this->window_handle.get()
		));

		//Create render pass
		//RESULT_TRY_MOVE(_render_pass, make<vk::render_pass>(logi_device, _swap_chain.format()));

		//Create depth image
		RESULT_TRY_MOVE(this->_depth_image, make<vk::depth_image>(logi_device, phys_device, this->_swap_chain.extent()));

		//Create framebuffers
		//_framebuffers.resize(_swap_chain.image_count());
		//for (size_t i = 0; i < _swap_chain.image_count(); i++) {
		//	RESULT_TRY_MOVE(_framebuffers[i], make<vk::framebuffer>(logi_device, _swap_chain.image_views()[i], _depth_image.view(), _render_pass, _swap_chain.extent()));
		//}


		for(sl::index_t i = 0; i < this->frames_in_flight; ++i) {
			//Create command buffers
			for(command_family_t j = 0; j < command_family::num_families; ++j) {
				RESULT_TRY_MOVE(this->_command_buffers[i][j], make<vk::command_buffer<Resources.size()>>(logi_device, phys_device, this->_command_pool_ptrs[j]));
				RESULT_TRY_MOVE(this->_command_buffer_semaphores[i][j], make<vk::semaphore>(logi_device, VK_SEMAPHORE_TYPE_TIMELINE));
				this->_command_buffer_semaphore_values[i][j] = std::make_unique<std::atomic<sl::uint64_t>>(0);
			}

			//Create fences & sempahores
			RESULT_TRY_MOVE(this->_graphics_fences[i], make<vk::fence>(logi_device));
			RESULT_TRY_MOVE(this->_acquisition_semaphores[i], make<vk::semaphore>(logi_device));
		}
		
		//Create submit sempahores
		this->_graphics_semaphores.reserve(this->_swap_chain.image_count());
		this->_pre_present_semaphores.reserve(this->_swap_chain.image_count());
		for(std::size_t i = 0; i < this->_swap_chain.image_count(); ++i) {
			RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(logi_device), graphics_semaphore);
			this->_graphics_semaphores.push_back(*std::move(graphics_semaphore));
			RESULT_VERIFY_UNSCOPED(make<vk::semaphore>(logi_device), pre_present_semaphore);
			this->_pre_present_semaphores.push_back(*std::move(pre_present_semaphore));
		}

		//Create generic semaphores
		for(std::size_t i = 0; i < command_family::num_families; ++i) {
			RESULT_TRY_MOVE(this->_generic_timeline_sempahores[i], make<vk::semaphore>(logi_device, VK_SEMAPHORE_TYPE_TIMELINE));
			this->_generic_semaphore_values[i] = 0;
		}


		//Create auxiliary objects
		auto create_aux = [this]<sl::index_t... Is>(sl::index_sequence_type<Is...>) -> result<void> {
			return ol::to_result((([this]() noexcept -> result<void> {
				if constexpr (std::is_same_v<typename sl::tuple_traits<decltype(auxiliary)>::template type_of_element<Is>, sl::empty_t>) {
					//_auxiliary[sl::index_constant<I>] = sl::empty_t{};
					return {};
				}
				RESULT_TRY_MOVE(auxiliary[sl::index_constant<Is>], (sl::type_of_pack_element_t<Is, timeline::setup<Ts>...>{}(*this)));
				return {};
			}) && ...));
		};
		RESULT_VERIFY(create_aux(sl::index_sequence_for_pack<timeline::setup<Ts>...>));
		
		D2D_INVOKE_ALL(this->timeline_callbacks(), on_swap_chain_updated_fn, *this);
			
		return {};
	}
}

namespace d2d {
	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    result<void> window<sl::tuple<Ts...>, Resources>::render() noexcept {
        //wait for rendering to finish last frame
		const sl::index_t frame_idx = this->frame_index();
        RESULT_VERIFY(this->graphics_fences()[frame_idx].wait());
        RESULT_VERIFY(this->graphics_fences()[frame_idx].reset());

		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_begin_fn, *this);

		timeline::state<Resources.size(), Resources> timeline_state{
			//.draw_buffer_offsets = sl::universal::make<typename timeline::state<Resources.size(), Resources>::draw_buffer_offsets_type>(
			//	Resources, 
			//	sl::functor::subscript<0>{}, 
			//	sl::functor::default_construct<sl::index_t>{}, 
			//	impl::draw_command_buffer_filtered_index_sequence<Resources.size(), Resources>{}
			//),

			.image_index = 0,
			.current_command_buffer_semaphore_values{}
		};
		
		constexpr auto exec = []<sl::index_t I>(window& win, timeline::state<Resources.size(), Resources>& state, sl::index_constant_type<I>) noexcept -> result<void> {
			return win.template execute_command<I>(state);
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, exec>{}(sl::index_sequence_for_pack<Ts...>, *this, timeline_state)));
        //((Ts{}(*this, timeline_state)), ...);


		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_end_fn, *this);

        //frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        ++this->_frame_count;
		//this->frame_count.fetch_add();
        return {};
    }


	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
	template<sl::index_t I>
    result<void> window<sl::tuple<Ts...>, Resources>::execute_command(timeline::state<Resources.size(), Resources>& state) noexcept {
		using timeline_type = typename sl::tuple_traits<sl::tuple<Ts...>>::template type_of_element<I>;
		return d2d::timeline::command<timeline_type>{}(*this, state, sl::universal::get<I>(auxiliary));
	}
}

namespace d2d {
	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
	template<typename TimelineCommandT>
    result<void> window<sl::tuple<Ts...>, Resources>::execute_command(timeline::state<Resources.size(), Resources>& state) noexcept {
		return d2d::timeline::command<TimelineCommandT>{}(*this, state, sl::empty_t{});
	}

	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
	template<typename TimelineCommandT>
    result<void> window<sl::tuple<Ts...>, Resources>::execute_command() noexcept {
		return execute_command(external_timeline_state());
	}
}


namespace d2d {
	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept {
        auto window_info_it = input::impl::glfw_window_map().find(window_ptr);
        if(window_info_it == input::impl::glfw_window_map().end()) [[unlikely]] return;
        window* win_ptr = static_cast<window*>(window_info_it->second.window_ptr);

        std::unique_lock<std::mutex> current_combo_lock(window_info_it->second.combo_mutex);
        window_info_it->second.current_combo.main_input() = code;
        window_info_it->second.current_combo.set(code, false);
        input::combination combo = (win_ptr->input_modifier_flags()[code] & input::modifier_flags::no_modifiers_allowed) ? input::combination{{}, code} : window_info_it->second.current_combo;
        window_info_it->second.current_combo.set(code, pressed);
        current_combo_lock.unlock();
        
        input::binding_map const& bind_map = pressed ? win_ptr->input_active_bindings() : win_ptr->input_inactive_bindings();
        auto event_set_it = bind_map.find(combo);
        if(event_set_it != bind_map.end()) goto invoke_event;

        event_set_it = bind_map.find(input::combination{{d2d::input::generic_code::any}, code});
        if(event_set_it != bind_map.end()) goto invoke_event;

        combo.main_input() = d2d::input::generic_code::any;
        event_set_it = bind_map.find(std::move(combo));
        if(event_set_it != bind_map.end()) goto invoke_event;

        event_set_it = bind_map.find(input::combination{{d2d::input::generic_code::any}, d2d::input::generic_code::any});
        if(event_set_it != bind_map.end()) goto invoke_event;

        return;
        
    invoke_event:
        input::category_flags_t category_flags = win_ptr->current_input_categories() & event_set_it->second.applicable_categories;
        //const unsigned long long category_flags = category_bitset.to_ullong();
        while(category_flags.any()) {
            input::category_id_t category_id = input::max_category_id - std::countl_zero(category_flags.to_ullong());
            input::event_id_t event_id = event_set_it->second.event_ids[category_id];
            auto event_fn_it = win_ptr->input_event_functions().find(input::categorized_event_t{event_id, category_id});
            if(event_fn_it != win_ptr->input_event_functions().end()) {
                std::invoke(event_fn_it->second, win_ptr, combo, pressed, input::categorized_event_t{event_id, category_id}, mouse_aux_data, glfwGetWindowUserPointer(window_ptr));
                //return;
            }
            category_flags.reset(category_id);
        }
    }


	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::kb_key_input(GLFWwindow* window_ptr, int key, int, int action, int) noexcept {
        if(key == GLFW_KEY_UNKNOWN) return;
        switch(action) {
        case GLFW_RELEASE:
        case GLFW_PRESS:
            break;
        default:
            return;
        }

        thread_pool().detach_task(std::bind(process_input, window_ptr, input::codes_map[key], static_cast<bool>(action), input::mouse_aux_t{}));
        //return process_input(window_ptr, input::codes_map[key], static_cast<bool>(action), input::mouse_aux_t{});
    }

	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept {
        auto window_info_it = input::impl::glfw_window_map().find(window_ptr);
        if(window_info_it == input::impl::glfw_window_map().end()) [[unlikely]] return;
        window* win_ptr = static_cast<window*>(window_info_it->second.window_ptr);

        std::function<input::text_event_function> const& text_input_fn = win_ptr->text_input_function();
        if(!text_input_fn) return;
        thread_pool().detach_task(std::bind(text_input_fn, win_ptr, codepoint));
        //std::invoke(text_input_fn, win_ptr, codepoint);
    }

	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, true, input::mouse_aux_t{pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, false, input::mouse_aux_t{pt2d{x, y}}));
        //return process_input(window_ptr, input::mouse_code::move, std::nullopt, pt2d{x, y});
    }

	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::mouse_button_input(GLFWwindow* window_ptr, int button, int action, int) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{}));
        //return process_input(window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{});
    }

	template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    void window<sl::tuple<Ts...>, Resources>::mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, true, input::mouse_aux_t{-pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, false, input::mouse_aux_t{-pt2d{x, y}}));
    }
}
