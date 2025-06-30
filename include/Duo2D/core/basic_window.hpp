#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <vulkan/vulkan_core.h>
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/display/surface.hpp"
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

#if __cpp_lib_constexpr_memory >= 202202L
#define __D2D_CONSTEXPR_UNIQUE_PTR constexpr
#else 
#define __D2D_CONSTEXPR_UNIQUE_PTR
#endif

namespace d2d::impl {
    constexpr static std::size_t frames_in_flight = 2;
}

namespace d2d {
    template<impl::renderable_like... Ts>
    struct basic_window : public vk::renderable_tuple<impl::frames_in_flight, Ts...> {
        constexpr static std::size_t frames_in_flight = impl::frames_in_flight;
    public:
        static result<basic_window> create(std::string_view title, std::size_t width, std::size_t height, std::shared_ptr<vk::instance> i) noexcept;

        basic_window() noexcept : basic_window(nullptr) {}
        result<void> initialize(std::shared_ptr<vk::logical_device> logi_device, std::shared_ptr<vk::physical_device> phys_device) noexcept;

    public:
        template<typename T>
        result<void> apply_changes() noexcept { return vk::renderable_tuple<frames_in_flight, Ts...>::template apply_changes<T>(_render_pass); }

        result<void> render() noexcept;

    public:    
        constexpr vk::logical_device  const& logical_device()  const noexcept { return *logi_device_ptr; }
        constexpr vk::physical_device const& physical_device() const noexcept { return *phys_device_ptr; }

        constexpr vk::surface         const& surface()         const noexcept { return _surface; }
        constexpr vk::swap_chain      const& swap_chain()      const noexcept { return _swap_chain; }
        constexpr vk::render_pass     const& render_pass()     const noexcept { return _render_pass; }
        constexpr vk::texture_map     const& texture_map()     const noexcept { return this->textures; }
        constexpr std::size_t                frame_index()     const noexcept { return frame_idx; }

 
    public:
        __D2D_CONSTEXPR_UNIQUE_PTR operator GLFWwindow*() const noexcept { return handle.get(); }
        __D2D_CONSTEXPR_UNIQUE_PTR explicit operator bool() const noexcept { return static_cast<bool>(handle); }


    private:
        
        basic_window(GLFWwindow* w) noexcept : 
            vk::renderable_tuple<frames_in_flight, Ts...>(),
            handle(w, {}), logi_device_ptr(), phys_device_ptr(), command_pool_ptr(),
            _surface(), _swap_chain(), _render_pass(),
            frame_idx(0), command_buffers{}, render_fences{}, frame_semaphores{}, submit_semaphores() {}
        friend vk::physical_device;
        
    private:

        std::unique_ptr<GLFWwindow, generic_functor<glfwDestroyWindow>> handle;
        std::shared_ptr<vk::logical_device> logi_device_ptr;
        std::shared_ptr<vk::physical_device> phys_device_ptr;
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
    };
}


#include "Duo2D/core/basic_window.inl"