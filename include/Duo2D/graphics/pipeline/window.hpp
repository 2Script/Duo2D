#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/instance.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/graphics/sync/fence.hpp"
#include "Duo2D/graphics/sync/semaphore.hpp"
#include "Duo2D/graphics/pipeline/renderable_buffer.hpp"

namespace d2d {
    struct window {
        static result<window> create(std::string_view title, std::size_t width, std::size_t height, const instance& i) noexcept;

        window() noexcept : window(nullptr) {}
        result<void> initialize(logical_device& logi_device, physical_device& phys_device) noexcept;

        result<void> render() noexcept;

        template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
        inline result<void> add(std::string_view name, R&& renderable);
        template<impl::RenderableType T>
        inline result<void> remove(std::string_view name);

    public:
        constexpr operator GLFWwindow*() const noexcept { return handle.get(); }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : 
            handle(w, glfwDestroyWindow), logi_device_ptr(nullptr), phys_device_ptr(nullptr),
            _surface(), _swap_chain(), _command_pool(), renderable_mapping(), data(),          
            frame_idx(0), command_buffers{}, render_fences{}, semaphores{} {}
        friend physical_device;
        
    private:
        constexpr static std::size_t frames_in_flight = 2;

        std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow)&> handle;
        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        //Decleration order matters: swap_chain MUST be destroyed before surface
        surface _surface;
        swap_chain _swap_chain;
        render_pass _render_pass;
        command_pool _command_pool;

        std::unordered_map<std::string, std::size_t> renderable_mapping;
        renderable_buffer<frames_in_flight, old_rect, styled_rect> data;
        
        std::size_t frame_idx;
        std::array<command_buffer, frames_in_flight> command_buffers;
        std::array<fence, frames_in_flight> render_fences;
        struct semaphore_type { enum { image_available, cmd_buffer_finished, num_semaphore_types }; };
        std::array<std::array<semaphore, frames_in_flight>, semaphore_type::num_semaphore_types> semaphores;

        constexpr static auto x = d2d::styled_rect::indices()[0];
    };
}

#include "Duo2D/graphics/pipeline/window.inl"