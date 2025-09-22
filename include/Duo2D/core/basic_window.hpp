#pragma once
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <result.hpp>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/moveable_atomic.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/event_function.hpp"
#include "Duo2D/input/event_int.hpp"
#include "Duo2D/input/map_types.hpp"
#include "Duo2D/input/modifier_flags.hpp"
#include "Duo2D/input/window_info.hpp"
#include "Duo2D/traits/asset_like.hpp"
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/interactable_like.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include "Duo2D/traits/same_as.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/display/depth_image.hpp"
#include "Duo2D/vulkan/display/display_format.hpp"
#include "Duo2D/vulkan/display/pixel_format.hpp"
#include "Duo2D/vulkan/display/present_mode.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/renderable_tuple.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/texture_map.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/vulkan/traits/renderable_data_traits.hpp"


#if __cpp_lib_constexpr_memory >= 202202L
#define __D2D_CONSTEXPR_UNIQUE_PTR constexpr
#else 
#define __D2D_CONSTEXPR_UNIQUE_PTR
#endif


namespace d2d::impl {
    constexpr static std::size_t frames_in_flight = 2;
    
    template<typename P>
    concept constructible_from_second_type_of = std::constructible_from<typename vk::impl::renderable_input_map<typename std::remove_cvref_t<P>::second_type>::value_type, P&&>;

    template<typename K, typename T, template<typename...> typename... Iters>
    concept not_convertible_to_iters = (!std::is_convertible_v<K&&, Iters<T>> && ...);
}


namespace d2d {
    template<typename... Ts>
    struct basic_window : public vk::renderable_tuple<impl::frames_in_flight, typename vk::renderable_data_traits<Ts...>::data_tuple_type> {
        constexpr static std::size_t frames_in_flight = impl::frames_in_flight;
    public:
        static result<basic_window> create(std::string_view title, unsigned int width, unsigned int height, std::shared_ptr<vk::instance> i) noexcept;

        basic_window() noexcept : basic_window(nullptr) {}
        result<void> initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device, std::shared_ptr<impl::font_data_map> font_data_map) noexcept;
    
        //TODO check that T is within container_data_tuple_type
        template<typename T>
        result<void> apply_changes() noexcept;
        template<impl::asset_like T>
        result<void> apply_changes() noexcept;
    private:
        template<typename T>
        result<void> apply_memory_changes() noexcept;
        template<typename T>
        result<void> apply_attributes() noexcept;
    
    private:
        result<bool> verify_swap_chain(VkResult fn_result, bool even_if_suboptimal) noexcept;

    public:
        template<impl::directly_renderable             T> constexpr bool has_changes() const noexcept;
        template<impl::renderable_container_like       T> constexpr bool has_changes() const noexcept;
        template<impl::renderable_container_tuple_like T> constexpr bool has_changes() const noexcept;
        template<impl::asset_like                      T> constexpr bool has_changes() const noexcept;
    private:
        template<impl::directly_renderable             T> constexpr bool has_changes(bool value) noexcept;
        template<impl::renderable_container_like       T> constexpr bool has_changes(bool value) noexcept;
        template<impl::renderable_container_tuple_like T> constexpr bool has_changes(bool value) noexcept;
        template<impl::asset_like                      T> constexpr bool has_changes(bool value) noexcept;


    public:
        result<void> render() noexcept;
    private:
        template<typename T>
        result<void> draw(std::size_t) const noexcept;


    public:    
        constexpr vk::logical_device  const& logical_device()  const noexcept { return *this->logi_device_ptr; }
        constexpr vk::physical_device const& physical_device() const noexcept { return *this->phys_device_ptr; }

        constexpr vk::surface                  const& surface()         const noexcept { return _surface; }
        constexpr vk::swap_chain               const& swap_chain()      const noexcept { return _swap_chain; }
        constexpr std::vector<vk::framebuffer> const& framebuffers()    const noexcept { return _framebuffers; }
        constexpr vk::depth_image              const& depth_image()     const noexcept { return _depth_image; }
        constexpr vk::render_pass              const& render_pass()     const noexcept { return _render_pass; }
        constexpr vk::texture_map              const& texture_map()     const noexcept { return this->textures; }
        constexpr std::size_t                         frame_index()     const noexcept { return frame_count.load() % impl::frames_in_flight; }
        constexpr extent2                      const& screen_size()     const noexcept { return _size; }

        //inline std::set<std::reference_wrapper<interactable>, impl::interactable_compare_functor>& interactables() noexcept { return interactable_refs; }
        inline std::weak_ptr<impl::font_data_map> font_data_map() const noexcept { return this->font_data_map_ptr; }

    public:
        __D2D_CONSTEXPR_UNIQUE_PTR operator GLFWwindow*() const noexcept { return handle.get(); }
        __D2D_CONSTEXPR_UNIQUE_PTR explicit operator bool() const noexcept { return static_cast<bool>(handle); }


        
    public:
        template<typename T> using key_type       = typename vk::renderable_data_traits<Ts...>::template key_type      <T, frames_in_flight>;
        template<typename T> using mapped_type    = typename vk::renderable_data_traits<Ts...>::template mapped_type   <T, frames_in_flight>;
        template<typename T> using value_type     = typename vk::renderable_data_traits<Ts...>::template value_type    <T, frames_in_flight>;
        template<typename T> using iterator       = typename vk::renderable_data_traits<Ts...>::template iterator      <T, frames_in_flight>;
        template<typename T> using const_iterator = typename vk::renderable_data_traits<Ts...>::template const_iterator<T, frames_in_flight>;
    public:
        template<typename R>
        std::pair<iterator<R>, bool> insert(const value_type<R>& value) noexcept;
        template<typename R>
        std::pair<iterator<R>, bool> insert(value_type<R>&& value) noexcept;

        template<impl::constructible_from_second_type_of P>
        std::pair<iterator<typename std::remove_cvref_t<P>::second_type>, bool> insert(P&& value) noexcept;

        template<typename T, typename V>
        std::pair<iterator<T>, bool> insert_or_assign(const key_type<T>& key, V&& value) noexcept;
        template<typename T, typename V>
        std::pair<iterator<T>, bool> insert_or_assign(key_type<T>&& key, V&& value) noexcept;
            
        template<typename T, typename... Args>
        std::pair<iterator<T>, bool> emplace(Args&&... args) noexcept;
        template<typename T, typename S, typename... Args> 
        std::pair<iterator<T>, bool> try_emplace(S&& str, Args&&... args) noexcept requires impl::not_convertible_to_iters<S, T, iterator, const_iterator>;
    private:
        template<typename T>
        void apply_insertion(key_type<T> key, T& inserted_value) noexcept;
    public:
        template<typename T>
        iterator<T> erase(iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> first, const_iterator<T> last) noexcept;
        template<typename T>
        std::size_t erase(key_type<T> const& key) noexcept;

    public:
        template<typename T>
        bool empty() const noexcept;
        template<typename T>
        std::size_t size() const noexcept;

    public:

    public:
        template<typename T> result<void> set_hidden(key_type<T> const& key, bool value) noexcept;
        template<typename T> result<void> toggle_hidden(key_type<T> const& key) noexcept;

        template<typename T> bool shown(key_type<T> const& key) noexcept;
        template<typename T> bool hidden(key_type<T> const& key) noexcept;


    public:
        constexpr input::category_flags_t const& current_input_categories() const noexcept { return category_flags; }
        constexpr input::category_flags_t      & current_input_categories()       noexcept { return category_flags; }
    public:
        constexpr input::binding_map const& input_active_bindings() const noexcept { return active_bindings; }
        constexpr input::binding_map      & input_active_bindings()       noexcept { return active_bindings; }
        constexpr input::binding_map const& input_inactive_bindings() const noexcept { return inactive_bindings; }
        constexpr input::binding_map      & input_inactive_bindings()       noexcept { return inactive_bindings; }
    public:
        constexpr input::event_fns_map const& event_functions() const noexcept { return event_fns; }
        constexpr input::event_fns_map      & event_functions()       noexcept { return event_fns; }
        constexpr std::function<input::text_event_function> const& text_input_function() const noexcept { return text_input_fn; }
        constexpr std::function<input::text_event_function>      & text_input_function()       noexcept { return text_input_fn; }
    public:
        constexpr std::array<input::modifier_flags_t, input::num_codes> const& input_modifier_flags() const noexcept { return modifier_flags; }
        constexpr std::array<input::modifier_flags_t, input::num_codes>      & input_modifier_flags()       noexcept { return modifier_flags; }

    private:
        static void process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept;
    private:
        static void kb_key_input(GLFWwindow* window_ptr, int key, int scancode, int action, int mods) noexcept;
        static void kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept; //not tied to a category
        static void mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)
        static void mouse_button_input(GLFWwindow* window_ptr, int button, int action, int mods) noexcept;
        static void mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)



    private:
        //using base_input_type = input::window_config;
        using base_tuple_type = vk::renderable_tuple<frames_in_flight, typename vk::renderable_data_traits<Ts...>::data_tuple_type>;

        template<impl::directly_renderable             T> constexpr vk::renderable_data<T, frames_in_flight>      & renderable_data_of()       noexcept;
        template<impl::directly_renderable             T> constexpr vk::renderable_data<T, frames_in_flight> const& renderable_data_of() const noexcept;
        template<impl::renderable_container_like       T> constexpr vk::impl::renderable_input_map<T>             & renderable_data_of()       noexcept;
        template<impl::renderable_container_like       T> constexpr vk::impl::renderable_input_map<T>        const& renderable_data_of() const noexcept;
        template<impl::renderable_container_tuple_like T> constexpr vk::impl::renderable_input_map<T>             & renderable_data_of()       noexcept;
        template<impl::renderable_container_tuple_like T> constexpr vk::impl::renderable_input_map<T>        const& renderable_data_of() const noexcept;
        template<impl::asset_like                      T> constexpr impl::asset_path_map<T>                       & renderable_data_of()       noexcept;
        template<impl::asset_like                      T> constexpr impl::asset_path_map<T>                  const& renderable_data_of() const noexcept;

        template<impl::interactable_like T> constexpr vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>>      & interactables_of()       noexcept;
        template<impl::interactable_like T> constexpr vk::impl::renderable_input_map<std::pair<std::reference_wrapper<T>, bool>> const& interactables_of() const noexcept;


        template<typename                              T> constexpr bool insert_children(key_type<T>    , T&          ) noexcept { return false; }
        template<impl::renderable_container_like       T> constexpr bool insert_children(key_type<T> key, T& container) noexcept;
        template<impl::renderable_container_tuple_like T> constexpr bool insert_children(key_type<T> key, T& tuple    ) noexcept;




    private:
        basic_window(GLFWwindow* w) noexcept : base_tuple_type(), 
            category_flags(static_cast<input::category_flags_t>(0b1) << input::category::system),
            active_bindings(), inactive_bindings(), event_fns(), text_input_fn(), modifier_flags{},
            renderable_container_datas(), interactable_refs(), asset_paths_tuple(),
            handle(w, {}), command_pool_ptr(),
            _surface(), _swap_chain(), _render_pass(), _size{},
            command_buffers{}, render_fences{}, frame_operation_semaphores{}, rendering_complete_semaphores(),
            frame_count{}, update_count(), focus_key(static_cast<std::size_t>(-1)) {}
        
    private:
        friend vk::physical_device;
        
        template<typename, std::size_t, template<typename, std::size_t...> typename>
        friend class renderable_container;
        template<typename, template<typename...> typename>
        friend class dynamic_renderable_container;

        template<typename WindowT>
        friend class application;

        //TEMP?
        friend struct d2d::input::defaults::interactable;


    private:
        //TODO: replace with lockable map so it can be thread-safe
        input::category_flags_t category_flags;
        input::binding_map active_bindings;
        input::binding_map inactive_bindings;
        input::event_fns_map event_fns;
        std::function<input::text_event_function> text_input_fn;
        std::array<input::modifier_flags_t, input::num_codes> modifier_flags;

    private:
        typename vk::renderable_data_traits<Ts...>::container_data_map_tuple_type renderable_container_datas;
        //std::vector<interactable*> interactable_ptrs;
        typename vk::renderable_data_traits<Ts...>::interactable_map_tuple_type interactable_refs;
        std::tuple<std::pair<impl::asset_path_map<font>, bool>, std::pair<impl::asset_path_map<texture>, bool>> asset_paths_tuple;

        std::unique_ptr<GLFWwindow, generic_functor<glfwDestroyWindow>> handle;
        std::shared_ptr<vk::command_pool> command_pool_ptr;

        constexpr static std::array<vk::pixel_format_info, 2> pixel_format_priority = {vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_SRGB)->second, vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_UNORM)->second};
        constexpr static vk::color_space_info default_color_space = vk::color_spaces.find(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)->second;
        //constexpr static std::pair<VkFormat, vk::pixel_format_info> default_pixel_format = *vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_SRGB);
        //constexpr static std::pair<VkColorSpaceKHR, vk::color_space_info> default_color_space = *vk::color_spaces.find(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
        //constexpr static vk::display_format default_display_format = {default_pixel_format.first, default_color_space.first, default_pixel_format.second, default_color_space.second};
        constexpr static std::array<vk::present_mode, static_cast<std::size_t>(vk::present_mode::num_present_modes)> present_mode_priority{vk::present_mode::mailbox, vk::present_mode::fifo, vk::present_mode::fifo_relaxed, vk::present_mode::immediate}; 
        //Decleration order matters: swap_chain MUST be destroyed before surface
        vk::surface _surface;
        vk::swap_chain _swap_chain;
        vk::depth_image _depth_image;
        std::vector<vk::framebuffer> _framebuffers;
        vk::render_pass _render_pass;
        extent2 _size;

        //std::size_t frame_idx;

        std::array<vk::command_buffer, frames_in_flight> command_buffers;
        std::array<vk::fence, frames_in_flight> render_fences;
        struct frame_operation { enum { image_acquired, /*cmd_buffer_finished,*/ num_semaphore_types }; };
        std::array<std::array<vk::semaphore, frames_in_flight>, frame_operation::num_semaphore_types> frame_operation_semaphores;
        std::vector<vk::semaphore> rendering_complete_semaphores;
        
        //TEMP
        moveable_atomic<std::size_t> frame_count, update_count;
        moveable_atomic<std::uint64_t> focus_key;

        constexpr static std::string_view container_child_prefix           = "__d2d_renderable_container";
        constexpr static std::string_view container_child_format_key       = "__d2d_renderable_container_{}__object{}";
        constexpr static std::string_view container_tuple_child_format_key = "__d2d_renderable_container_tuple_{}__object{}";
    };
}


#include "Duo2D/core/basic_window.inl"