#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <functional>
#include <memory>
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/instance.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline.hpp"
#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/sync/fence.hpp"
#include "Duo2D/graphics/sync/semaphore.hpp"

namespace d2d {
    struct window {
        static result<window> create(std::string_view title, std::size_t width, std::size_t height, const instance& i) noexcept;

        window() noexcept : window(nullptr) {}
        result<void> initialize(logical_device& logi_device, physical_device& phys_device) noexcept;
        result<void> render() noexcept;

    public:
        constexpr operator GLFWwindow*() const noexcept { return handle.get(); }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : 
            handle(w, glfwDestroyWindow), logi_device_ptr(nullptr), phys_device_ptr(nullptr),
            _surface(), _swap_chain(), _pipeline(), _command_pool(), frame_idx(0),
            command_buffers{}, render_fences{}, image_available_semaphores{}, cmd_buffer_finished_semaphores{} {}
        friend physical_device;
        friend command_buffer;
        
    private:
        std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow)&> handle;
        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        //Decleration order matters: swap_chain MUST be destroyed before surface
        surface _surface;
        swap_chain _swap_chain;
        render_pass _render_pass;
        pipeline _pipeline;
        command_pool _command_pool;
        
        constexpr static std::size_t frames_in_flight = 2;
        std::size_t frame_idx;
        std::array<command_buffer, frames_in_flight> command_buffers;
        std::array<fence, frames_in_flight> render_fences;
        std::array<semaphore, frames_in_flight> image_available_semaphores, cmd_buffer_finished_semaphores;
    };
}