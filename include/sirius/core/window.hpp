#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/core/render_instance.fwd.hpp"
#include "sirius/core/window.fwd.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/input/category.hpp"
#include "sirius/input/code.hpp"
#include "sirius/core/error.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/map_types.hpp"
#include "sirius/input/modifier_flags.hpp"
#include "sirius/vulkan/core/instance.hpp"
#include "sirius/vulkan/display/surface.hpp"
#include "sirius/vulkan/display/swap_chain.hpp"
#include "sirius/vulkan/display/depth_image.hpp"


namespace acma {
    class window {
	public:
        constexpr window() noexcept :
            category_flags(static_cast<input::category_flags_t>(0b1) << input::category::system),
            active_bindings(), inactive_bindings(), event_fns(), text_input_fn(), modifier_flags{},
			window_handle(), _surface{}, _swap_chain{}, _depth_image{}, _size{} {}
	public:
        inline static result<window> create(
			acma::sz2u32 size, std::string_view title
		) noexcept;

		inline result<void> initialize(
			std::shared_ptr<vk::logical_device> logi_device, 
			vk::physical_device* phys_device
		) noexcept;
		
	public:
		inline result<bool> verify_swap_chain(
			VkResult fn_result, 
			std::shared_ptr<vk::logical_device> logi_device, 
			vk::physical_device* phys_device,
			bool even_if_suboptimal
		) noexcept;

	public:
		constexpr vk::surface     const& surface           (this auto const& self) noexcept { return self._surface; }
		constexpr vk::swap_chain  const& swap_chain        (this auto const& self) noexcept { return self._swap_chain; }
		constexpr vk::depth_image const& depth_image       (this auto const& self) noexcept { return self._depth_image; }
		
		constexpr extent2 screen_size() const noexcept { return _size; }
    
	public:
        constexpr auto&& current_input_categories(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.category_flags); }
        constexpr auto&& input_active_bindings   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.active_bindings); }
        constexpr auto&& input_inactive_bindings (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.inactive_bindings); }
        constexpr auto&& input_event_functions   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.event_fns); }
        constexpr auto&& text_input_function     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.text_input_fn); }
        constexpr auto&& input_modifier_flags    (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.modifier_flags); }

    private:
        inline static void process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept;
    private:
        inline static void kb_key_input(GLFWwindow* window_ptr, int key, int scancode, int action, int mods) noexcept;
        inline static void kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept; //not tied to a category
        inline static void mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)
        inline static void mouse_button_input(GLFWwindow* window_ptr, int button, int action, int mods) noexcept;
        inline static void mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)

	private: 
    	template<typename, bool>
		friend class application;

    	template<typename TimelineT, auto BufferConfigs, auto AssetHeapConfigs>
		requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
		friend class render_instance;
		
	private:
        input::category_flags_t category_flags;
        input::binding_map active_bindings;
        input::binding_map inactive_bindings;
        input::event_fns_map event_fns;
        std::function<input::text_event_function> text_input_fn;
        std::array<input::modifier_flags_t, input::num_codes> modifier_flags;
    private:
        std::unique_ptr<GLFWwindow, sl::functor::generic_stateless<glfwDestroyWindow>> window_handle;
		vk::surface _surface;
		vk::swap_chain _swap_chain;
		vk::depth_image _depth_image;
		extent2 _size;

    };
}


#include "sirius/core/window.inl"