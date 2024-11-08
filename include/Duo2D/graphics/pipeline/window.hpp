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
        result<void> initialize(logical_device& logi_deivce, physical_device& phys_device) noexcept;
        result<void> render() const noexcept;

    public:
        constexpr operator GLFWwindow*() const noexcept { return handle.get(); }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : 
            handle(w, glfwDestroyWindow), 
            _surface(), _swap_chain(), _pipeline(), _command_pool(), _command_buffer(),
            render_fence(), image_available(), cmd_buffer_finished() {}
        friend physical_device;
        friend command_buffer;
        
    private:
        std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow)&> handle;
        logical_device* device_ptr;
        //Decleration order matters: swap_chain MUST be destroyed before surface
        surface _surface;
        swap_chain _swap_chain;
        render_pass _render_pass;
        pipeline _pipeline;
        command_pool _command_pool;
        command_buffer _command_buffer;
        
        fence render_fence;
        semaphore image_available, cmd_buffer_finished;
    };
}