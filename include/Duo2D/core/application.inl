#pragma once
#include "Duo2D/core/application.hpp"

#include <atomic>
#include <memory>
#include <utility>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Duo2D/core/error.hpp"
#include "Duo2D/core/make.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"

namespace d2d {
    template<typename WindowT>
    result<application<WindowT>> application<WindowT>::create(std::string_view name) noexcept {
        // Set application info
        VkApplicationInfo app_info{
            VK_STRUCTURE_TYPE_APPLICATION_INFO, //sType
            nullptr,                            //pNext
            name.data(),              //App Name
            VK_MAKE_VERSION(1, 0, 0), //App Version (temp)
            "Duo2D",                  //Engine Name
            VK_MAKE_VERSION(0, 0, 1), //Engine Version
            VK_API_VERSION_1_3        //API Version
        };
        
        // Initialize GLFW
        //if(int code; application::glfw_init.count++ == 0 && !glfwInit() && (code = glfwGetError(nullptr)))
        //    return static_cast<errc>(code);

        application ret{};
        RESULT_VERIFY(ret.glfw_init.initialize(impl::application_count(), []() noexcept -> result<void> {
            if(int code; !glfwInit()) {
                if((code = glfwGetError(nullptr)))
                    return static_cast<errc>(code | 0x00010000);
                return errc::os_window_error;
            }
            return {};
        }));

        // Check for vulkan support
        if(!glfwVulkanSupported()) 
            return error::vulkan_not_supported;

        // Create instance
        vk::instance i;
        RESULT_TRY_MOVE(i, make<vk::instance>(app_info));
        ret.instance_ptr = std::make_shared<vk::instance>(std::move(i));

        ret.phys_device_ptr = std::make_shared_for_overwrite<vk::physical_device>();
        ret.logi_device_ptr = std::make_shared_for_overwrite<vk::logical_device>();
        ret.font_data_map_ptr = std::make_shared_for_overwrite<impl::font_data_map>();
        ret.name = name;

        ret.should_be_open.store(true, std::memory_order_relaxed);

        return ret;
    }
}


namespace d2d {
    template<typename WindowT>
    result<std::set<vk::physical_device>> application<WindowT>::devices() const noexcept {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(*instance_ptr, &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::vector<VkPhysicalDevice> devices(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(*instance_ptr, &device_count, devices.data()));

        //Create dummy window
        RESULT_TRY_MOVE_UNSCOPED(window dummy, make<window>("dummy", 1600, 900, instance_ptr), win);


        std::set<vk::physical_device> ret{};
        for(VkPhysicalDevice device_handle : devices) {
            result<vk::physical_device> d = make<vk::physical_device>(device_handle, dummy.surface());
            if(!d.has_value()) return d.error();
            ret.insert(*std::move(d));
        }

        return ret;
    }


    template<typename WindowT>
    result<void> application<WindowT>::initialize_device() noexcept {
        //Create logical device
        vk::logical_device l;
        RESULT_TRY_MOVE(l, make<vk::logical_device>(phys_device_ptr));
        logi_device_ptr = std::make_shared<vk::logical_device>(std::move(l));

        return {};
    }
}


namespace d2d {
    template<typename WindowT>
    result<WindowT*> application<WindowT>::add_window(std::string_view title) noexcept {
        if(!logi_device_ptr)
            return error::device_not_initialized;

        
        result<WindowT> w = make<WindowT>(title, 1600, 900, instance_ptr);
        if(!w.has_value()) return w.error();
        RESULT_VERIFY(w->initialize(logi_device_ptr, phys_device_ptr, font_data_map_ptr));

        auto new_window = windows.emplace(title, *std::move(w));
        WindowT* new_window_ptr = &(new_window.first->second);
        input::impl::glfw_window_map().try_emplace(static_cast<GLFWwindow*>(*new_window_ptr), input::combination{}, new_window_ptr);

        if(!new_window.second) 
            return error::window_already_exists;
        return new_window_ptr;
    }


    template<typename WindowT>
    result<void> application<WindowT>::remove_window(std::string_view title) noexcept {
        if (auto it = windows.find(std::string(title)); it != windows.end()) {
            windows.erase(it);
            input::impl::glfw_window_map().erase(static_cast<GLFWwindow*>(it->second));
            return {};
        }
 
        return error::window_not_found;
    }
}

namespace d2d {
    template<typename WindowT>
    result<WindowT*> application<WindowT>::add_window() noexcept {
        return add_window(name);
    }

    template<typename WindowT>
    result<void> application<WindowT>::remove_window() noexcept {
        return remove_window(name);
    }
}


namespace d2d {
    template<typename WindowT>
    bool application<WindowT>::open() const noexcept {
        return should_be_open.load(std::memory_order_relaxed);
    }

    template<typename WindowT>
    void application<WindowT>::poll_events() noexcept {
        glfwPollEvents();
        for (auto& w : windows)
            if(!glfwWindowShouldClose(w.second))
                return;
        should_be_open.store(false, std::memory_order_relaxed);
    }

    template<typename WindowT>
    result<void> application<WindowT>::render() noexcept {
        for (auto& w : windows) 
            if(auto r = w.second.render(); !r.has_value()) 
                return r.error();
        return {};
    }

    template<typename WindowT>
    result<void> application<WindowT>::join() noexcept {
        __D2D_VULKAN_VERIFY(vkDeviceWaitIdle(*logi_device_ptr));
        return  {};
    }
}

//namespace d2d {
//    template<typename WindowT>
//    std::atomic_int64_t application<WindowT>::instance_count{};
//}