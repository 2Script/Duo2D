#pragma once
#include "Duo2D/core/window.fwd.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>

#include "Duo2D/timeline/setup.hpp"
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/map_types.hpp"
#include "Duo2D/input/modifier_flags.hpp"
#include "Duo2D/timeline/state.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/core/render_process.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"


namespace d2d {
    template<typename... Ts, auto Resources> requires impl::is_resource_table_v<decltype(Resources)>
    struct window<sl::tuple<Ts...>, Resources> : public render_process<Resources.size(), Resources> {
		using render_process_type = render_process<Resources.size(), Resources>;
	public:
        static result<window> create(std::string_view title, unsigned int width, unsigned int height, std::shared_ptr<vk::instance> i) noexcept;
		result<void> initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device) noexcept;

        window() noexcept :
			render_process_type(), 
			_external_timeline_state{}, auxiliary{},
            category_flags(static_cast<input::category_flags_t>(0b1) << input::category::system),
            active_bindings(), inactive_bindings(), event_fns(), text_input_fn(), modifier_flags{} {}

    public:
        result<void> render() noexcept;
	public:
		template<typename TimelineCommandT>
		result<void> execute_command(timeline::state<Resources.size(), Resources>& state) noexcept;
		template<typename TimelineCommandT>
		result<void> execute_command() noexcept;
	private:
		template<sl::index_t I>
		result<void> execute_command(timeline::state<Resources.size(), Resources>& state) noexcept;

	public:
		constexpr auto&& external_timeline_state(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._external_timeline_state); }
		constexpr auto&& timeline_callbacks     (this auto&& self) noexcept {return sl::forward_like<decltype(self)>(self._timeline_callbacks); }

    public:
        constexpr auto&& current_input_categories(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.category_flags); }
        constexpr auto&& input_active_bindings   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.active_bindings); }
        constexpr auto&& input_inactive_bindings (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.inactive_bindings); }
        constexpr auto&& input_event_functions   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.event_fns); }
        constexpr auto&& text_input_function     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.text_input_fn); }
        constexpr auto&& input_modifier_flags    (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.modifier_flags); }

    private:
        static void process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept;
    private:
        static void kb_key_input(GLFWwindow* window_ptr, int key, int scancode, int action, int mods) noexcept;
        static void kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept; //not tied to a category
        static void mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)
        static void mouse_button_input(GLFWwindow* window_ptr, int button, int action, int mods) noexcept;
        static void mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)

    public:

    private:
        friend vk::physical_device;

        template<typename WindowT>
        friend class application;
 
	private:
		timeline::state<Resources.size(), Resources> _external_timeline_state;
		sl::tuple<typename std::invoke_result_t<timeline::setup<Ts>, render_process<Resources.size(), Resources> const&>::value_type...> auxiliary;

    private:
        input::category_flags_t category_flags;
        input::binding_map active_bindings;
        input::binding_map inactive_bindings;
        input::event_fns_map event_fns;
        std::function<input::text_event_function> text_input_fn;
        std::array<input::modifier_flags_t, input::num_codes> modifier_flags;

    };
}


#include "Duo2D/core/window.inl"