#pragma once
#include <map>
#include <string_view>
#include <set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Duo2D/error.hpp"
//#include "Duo2D/graphics/window.hpp"
#include "Duo2D/hardware/device/device_info.hpp"
#include "Duo2D/hardware/display/display_format.hpp"


namespace d2d {
    class application {
    public:
        application() noexcept = default;
        ~application() noexcept;

        static result<application> create(std::string_view name) noexcept;

        application(application&&) noexcept;
        application& operator=(application&&) noexcept;
        //copy consutrct/assign is implicitly deleted because window is not copyable


    public:
        result<std::set<device_info>> devices() const noexcept;

        const device_info& selected_device() const& noexcept { return physical_device; }
        device_info& selected_device() & noexcept { return physical_device; }

        result<void> initialize_device() noexcept;


    public: 
        result<void> add_window(std::string_view title) noexcept;
        result<void> remove_window(std::string_view title) noexcept;

    private:
        struct window {
            GLFWwindow* handle;
            VkSurfaceKHR surface;
        };

        
    private:
        VkInstance vulkan_instance = VK_NULL_HANDLE;
        device_info physical_device;
        VkDevice logical_device = VK_NULL_HANDLE;
        display_format device_format;
        present_mode device_present_mode;

        std::array<VkQueue, queue_family::num_families> queues;


        std::map<std::string, window> windows;
    private:
        inline static bool glfw_init = false;

    private:
        constexpr static impl::display_format_table_t display_format_table = impl::all_display_formats();
    };
}


namespace d2d {
    result<application> make_application(std::string_view name) noexcept;
}