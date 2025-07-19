#pragma once
#include <GLFW/glfw3.h>
#include <atomic>
#include <map>
#include <memory>
#include <string_view>
#include <set>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/vulkan/core/instance.hpp"
#include "Duo2D/traits/instance_tracker.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/core/window.hpp"


namespace d2d {
    class application {
    public:
        application() noexcept = default;
        static result<application> create(std::string_view name) noexcept;


    public:
        result<std::set<vk::physical_device>> devices() const noexcept;

        const vk::physical_device& selected_device() const& noexcept { return *phys_device_ptr; }
        vk::physical_device& selected_device() & noexcept { return *phys_device_ptr; }

        result<void> initialize_device() noexcept;


    public:
        //TODO return reference to window instead of pointer
        result<window*> add_window(std::string_view title) noexcept;
        result<void> remove_window(std::string_view title) noexcept;

        result<window*> add_window() noexcept;
        result<void> remove_window() noexcept;

    public:
        bool open() const noexcept;
        result<void> render() noexcept;
        result<void> join() noexcept;

        
    private:
        std::shared_ptr<vk::instance>        instance_ptr;
        std::shared_ptr<vk::physical_device> phys_device_ptr;
        std::shared_ptr<vk::logical_device>  logi_device_ptr;
        std::shared_ptr<impl::font_data_map> font_data_map_ptr;
        std::string name;

        //ORDER MATTERS: glfw must be terminated after all windows have been destroyed
        impl::instance_tracker<void, glfwTerminate> glfw_init;
        std::map<std::string, window> windows;
    private:
        //inline static std::atomic_int64_t instance_count;
    };
}

namespace d2d::impl {
    inline std::atomic_int64_t& application_count() {
        static std::atomic_int64_t count{};
        return count;
    }
}