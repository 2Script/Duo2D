#pragma once
#include <GLFW/glfw3.h>
#include <map>
#include <string_view>
#include <set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/instance.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/pipeline/window.hpp"


namespace d2d {
    class application {
    public:
        application() noexcept = default;
        static result<application> create(std::string_view name) noexcept;


    public:
        result<std::set<physical_device>> devices() const noexcept;

        const physical_device& selected_device() const& noexcept { return phys_device; }
        physical_device& selected_device() & noexcept { return phys_device; }

        result<void> initialize_device() noexcept;


    public: 
        result<void> add_window(std::string_view title) noexcept;
        result<void> remove_window(std::string_view title) noexcept;

        result<void> add_window() noexcept;
        result<void> remove_window() noexcept;

    public:
        result<void> render() noexcept;

        
    private:
        instance vk_instance;
        physical_device phys_device;
        logical_device logi_device;
        std::string name;

        std::map<std::string, window> windows;
    private:
        inline static bool glfw_init = false;
    };
}