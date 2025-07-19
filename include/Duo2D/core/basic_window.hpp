#pragma once
#include <GLFW/glfw3.h>
#include <concepts>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
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
        static result<basic_window> create(std::string_view title, std::size_t width, std::size_t height, std::shared_ptr<vk::instance> i) noexcept;

        basic_window() noexcept : basic_window(nullptr) {}
        result<void> initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device, std::shared_ptr<impl::font_data_map> font_data_map) noexcept;
    
        template<typename T>
        result<void> apply_changes() noexcept;
        template<typename T>
        constexpr bool const& has_changes() const noexcept;
        template<typename T>
        constexpr bool      & has_changes()       noexcept;


    public:
        result<void> render() noexcept;


    public:    
        constexpr vk::logical_device  const& logical_device()  const noexcept { return *this->logi_device_ptr; }
        constexpr vk::physical_device const& physical_device() const noexcept { return *this->phys_device_ptr; }

        constexpr vk::surface                 const& surface()       const noexcept { return _surface; }
        constexpr vk::swap_chain              const& swap_chain()    const noexcept { return _swap_chain; }
        constexpr vk::render_pass             const& render_pass()   const noexcept { return _render_pass; }
        constexpr vk::texture_map             const& texture_map()   const noexcept { return this->textures; }
        constexpr std::size_t                        frame_index()   const noexcept { return frame_idx; }
        inline    std::weak_ptr<impl::font_data_map> font_data_map() const noexcept { return this->font_data_map_ptr; }

 
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
            
        template<typename T, typename... Args>
        std::pair<iterator<T>, bool> emplace(Args&&... args) noexcept;
        template<typename T, typename S, typename... Args> 
        std::pair<iterator<T>, bool> try_emplace(S&& str, Args&&... args) noexcept requires impl::not_convertible_to_iters<S, T, iterator, const_iterator>;
    public:
        template<typename T>
        iterator<T> erase(iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> pos) noexcept;
        template<typename T>
        iterator<T> erase(const_iterator<T> first, const_iterator<T> last) noexcept;
        template<typename T>
        std::size_t erase(std::string_view key) noexcept;

    public:
        template<typename T>
        bool empty() const noexcept;
        template<typename T>
        std::size_t size() const noexcept;


    private:
        using base_type = vk::renderable_tuple<frames_in_flight, typename vk::renderable_data_traits<Ts...>::data_tuple_type>;

        template<impl::directly_renderable       T> constexpr vk::renderable_data<T, frames_in_flight>      & renderable_data_of()       noexcept;
        template<impl::directly_renderable       T> constexpr vk::renderable_data<T, frames_in_flight> const& renderable_data_of() const noexcept;
        template<impl::renderable_container_like T> constexpr vk::impl::renderable_input_map<T>             & renderable_data_of()       noexcept;
        template<impl::renderable_container_like T> constexpr vk::impl::renderable_input_map<T>        const& renderable_data_of() const noexcept;


        template<impl::directly_renderable       T> constexpr bool insert_children(iterator<T>     ) noexcept { return false; }
        template<impl::renderable_container_like T> constexpr bool insert_children(iterator<T> iter) noexcept;




    private:
        basic_window(GLFWwindow* w) noexcept : base_type(),
            handle(w, {}),
            _surface(), _swap_chain(), _render_pass(),
            frame_idx(0), command_buffers{}, render_fences{}, frame_semaphores{}, submit_semaphores() {}
        friend vk::physical_device;
        
    private:
        template<typename, impl::directly_renderable, template<typename...> typename>
        friend class dynamic_renderable_container;

    private:
        typename vk::renderable_data_traits<Ts...>::container_data_tuple_type renderable_container_datas;

        std::unique_ptr<GLFWwindow, generic_functor<glfwDestroyWindow>> handle;
        std::shared_ptr<vk::command_pool> command_pool_ptr;

        //Decleration order matters: swap_chain MUST be destroyed before surface
        vk::surface _surface;
        vk::swap_chain _swap_chain;
        vk::render_pass _render_pass;

        std::size_t frame_idx;
        std::array<vk::command_buffer, frames_in_flight> command_buffers;
        std::array<vk::fence, frames_in_flight> render_fences;
        struct semaphore_type { enum { image_available, /*cmd_buffer_finished,*/ num_semaphore_types }; };
        std::array<std::array<vk::semaphore, frames_in_flight>, semaphore_type::num_semaphore_types> frame_semaphores;
        std::vector<vk::semaphore> submit_semaphores;

        //constexpr static std::size_t x = decltype(data)::template static_offsets<clone_rect>()[buffer_data_type::index];
        //constexpr static std::size_t y = renderable_data<clone_rect, 2>::static_index_data_bytes.size();
        constexpr static std::string_view container_child_format_key = "__d2d_renderable_container_{}__object_{}";
    };
}


#include "Duo2D/core/basic_window.inl"