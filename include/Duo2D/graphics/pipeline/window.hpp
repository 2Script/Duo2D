#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
#include <memory>
#include "Duo2D/graphics/pipeline/instance.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"

namespace d2d {
    struct window {
        static result<window> create(std::string_view title, std::size_t width, std::size_t height, const instance& i) noexcept;

        window() noexcept : handle(nullptr, glfwDestroyWindow), window_surface(), window_swap_chain() {}
        result<void> initialize_swap(logical_device& logi_deivce, physical_device& phys_device) noexcept;

    public:
        constexpr operator GLFWwindow*() const noexcept { return handle.get(); }
        constexpr explicit operator bool() const noexcept { return static_cast<bool>(handle); }

    private:
        window(GLFWwindow* w) noexcept : handle(w, glfwDestroyWindow), window_surface(), window_swap_chain() {}
        friend physical_device;
        
    private:
        std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow)&> handle;
        //Decleration order matters: swap_chain MUST be destroyed before surface
        surface window_surface;
        swap_chain window_swap_chain;
    };
}