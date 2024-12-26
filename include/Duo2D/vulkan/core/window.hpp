#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/prim/debug_rect.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/vulkan/memory/descriptor_set.hpp"
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/vulkan/sync/fence.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"

namespace d2d {
    struct window {
        static result<window> create(std::string_view title, std::size_t width, std::size_t height, const instance& i) noexcept;

        window() noexcept : window(nullptr) {}
        result<void> initialize(logical_device& logi_device, physical_device& phys_device) noexcept;

        result<void> render() noexcept;

    public:
        template<typename R> using value_type = std::pair<const std::string, R>;

        template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
        bool insert(const value_type<R>& value) noexcept;
        template<typename P> requires std::is_constructible_v<value_type<typename std::remove_cvref_t<P>::second_type>, P&&>
        bool insert(P&& value) noexcept;
        template<typename R> requires impl::RenderableType<std::remove_cvref_t<R>>
        bool insert(value_type<R>&& value) noexcept;
        template<typename T, typename S, typename... Args> requires std::is_constructible_v<std::string, S&&>
        bool emplace(S&& str, Args&&... args) noexcept;

        template<typename T>
        std::size_t erase(std::string_view key) noexcept;

        template<typename T>
        result<void> apply(bool shrink = false) noexcept;

    public:
        constexpr operator GLFWwindow*() const noexcept { return handle.get(); }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : 
            handle(w, glfwDestroyWindow), logi_device_ptr(nullptr), phys_device_ptr(nullptr),
            _surface(), _swap_chain(), data(), renderable_mapping(), 
            frame_idx(0), _command_pool(), command_buffers{}, render_fences{}, semaphores{} {}
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

        renderable_buffer<frames_in_flight, styled_rect, debug_rect, clone_rect> data;
        std::unordered_map<std::string, std::size_t> renderable_mapping;
        
        std::size_t frame_idx;
        command_pool _command_pool;
        std::array<command_buffer, frames_in_flight> command_buffers;
        std::array<fence, frames_in_flight> render_fences;
        struct semaphore_type { enum { image_available, cmd_buffer_finished, num_semaphore_types }; };
        std::array<std::array<semaphore, frames_in_flight>, semaphore_type::num_semaphore_types> semaphores;
    };
}

#include "Duo2D/vulkan/core/window.inl"