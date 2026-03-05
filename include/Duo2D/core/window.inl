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
    result<window>    window::
	create(
		std::shared_ptr<vk::instance> instance,
		d2d::sz2u32 size, std::string_view title
	) noexcept {

        //Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        window ret{};
		ret.window_handle = std::unique_ptr<GLFWwindow, sl::functor::generic_stateless<glfwDestroyWindow>>(glfwCreateWindow(size.width(), size.height(), title.data(), nullptr, nullptr));
        __D2D_GLFW_VERIFY(ret.window_handle);

        //Set callbacks
        glfwSetKeyCallback(ret.window_handle.get(), kb_key_input);
        glfwSetCharCallback(ret.window_handle.get(), kb_text_input);
        glfwSetCursorPosCallback(ret.window_handle.get(), mouse_move);
        glfwSetMouseButtonCallback(ret.window_handle.get(), mouse_button_input);
        glfwSetScrollCallback(ret.window_handle.get(), mouse_scroll);

        //Create surface
        RESULT_TRY_MOVE(ret._surface, make<vk::surface>(ret.window_handle.get(), instance));

        return std::move(ret);
    }



    result<void>    window::
	initialize(
		std::shared_ptr<vk::logical_device> logi_device, 
		std::shared_ptr<vk::physical_device> phys_device
	) noexcept {
		//Create swap chain
		RESULT_TRY_MOVE(_swap_chain, make<vk::swap_chain>(
			logi_device, 
			phys_device,
			_surface,
			*window_handle
		));

		//Create depth image
		RESULT_TRY_MOVE(_depth_image, make<vk::depth_image>(logi_device, phys_device, _swap_chain.extent()));


        //Verify window size
		d2d::sz2<int> actual_size{};
        glfwGetWindowSize(window_handle.get(), &actual_size.width(), &actual_size.height());
        _size = static_cast<d2d::extent2>(actual_size);

		return {};
	}
}


namespace d2d {
	result<bool>    window::
	verify_swap_chain(
		VkResult fn_result, 
		std::shared_ptr<vk::logical_device> logi_device, 
		std::shared_ptr<vk::physical_device> phys_device,
		bool even_if_suboptimal
	) noexcept {
		switch(fn_result) {
		case VK_SUCCESS:
			return false;
		case VK_SUBOPTIMAL_KHR:
			if(!even_if_suboptimal) return false;
			[[fallthrough]];
		case VK_ERROR_OUT_OF_DATE_KHR: {
			vkDeviceWaitIdle(*logi_device);
			
			RESULT_VERIFY(_swap_chain.reset(logi_device, phys_device, _surface, *window_handle));

			RESULT_TRY_MOVE(_depth_image, make<vk::depth_image>(logi_device, phys_device, _swap_chain.extent()));
			return true;
		}
		default: 
			return static_cast<errc>(__D2D_VKRESULT_TO_ERRC(fn_result));
		}
	}
}


namespace d2d {
    void window::process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept {
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


    void window::kb_key_input(GLFWwindow* window_ptr, int key, int, int action, int) noexcept {
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

    void window::kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept {
        auto window_info_it = input::impl::glfw_window_map().find(window_ptr);
        if(window_info_it == input::impl::glfw_window_map().end()) [[unlikely]] return;
        window* win_ptr = static_cast<window*>(window_info_it->second.window_ptr);

        std::function<input::text_event_function> const& text_input_fn = win_ptr->text_input_function();
        if(!text_input_fn) return;
        thread_pool().detach_task(std::bind(text_input_fn, win_ptr, codepoint));
        //std::invoke(text_input_fn, win_ptr, codepoint);
    }

    void window::mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, true, input::mouse_aux_t{pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::move, false, input::mouse_aux_t{pt2d{x, y}}));
        //return process_input(window_ptr, input::mouse_code::move, std::nullopt, pt2d{x, y});
    }

    void window::mouse_button_input(GLFWwindow* window_ptr, int button, int action, int) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{}));
        //return process_input(window_ptr, input::codes_map[button], static_cast<bool>(action), input::mouse_aux_t{});
    }

    void window::mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept {
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, true, input::mouse_aux_t{-pt2d{x, y}}));
        thread_pool().detach_task(std::bind(process_input, window_ptr, input::mouse_code::scroll, false, input::mouse_aux_t{-pt2d{x, y}}));
    }
}
