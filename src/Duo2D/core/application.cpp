#include "Duo2D/core/application.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>

#include "Duo2D/core/error.hpp"
#include "Duo2D/core/make.hpp"

namespace d2d {
    result<application> application::create(std::string_view name) noexcept {
        // Set application info
        VkApplicationInfo app_info{
            VK_STRUCTURE_TYPE_APPLICATION_INFO, //sType
            nullptr,                            //pNext
            name.data(),              //App Name
            VK_MAKE_VERSION(1, 0, 0), //App Version (temp)
            "Duo2D",                  //Engine Name
            VK_MAKE_VERSION(1, 0, 0), //Engine Version (temp)
            VK_API_VERSION_1_2        //API Version
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
        ret.name = name;

        return ret;
    }
}


namespace d2d {
    result<std::set<vk::physical_device>> application::devices() const noexcept {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(*instance_ptr, &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::vector<VkPhysicalDevice> devices(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(*instance_ptr, &device_count, devices.data()));

        //Create dummy window
        //__D2D_TRY_MAKE(window dummy, make<window>("dummy", 1280, 720, vk_instance), var_name);
        auto var_name = make<window>("dummy", 1280, 720, instance_ptr);
        if (!var_name.has_value())
          return var_name.error();
        window dummy = *std ::move(var_name);


        std::set<vk::physical_device> ret{};
        for(VkPhysicalDevice device_handle : devices) {
            result<vk::physical_device> d = make<vk::physical_device>(device_handle, dummy);
            if(!d.has_value()) return d.error();
            ret.insert(*std::move(d));
        }

        return ret;
    }


    result<void> application::initialize_device() noexcept {
        //Create logical device
        vk::logical_device l;
        RESULT_TRY_MOVE(l, make<vk::logical_device>(phys_device_ptr));
        logi_device_ptr = std::make_shared<vk::logical_device>(std::move(l));

        return {};
    }
}


namespace d2d {
    result<window*> application::add_window(std::string_view title) noexcept {
        if(!logi_device_ptr)
            return error::device_not_initialized;

        
        result<window> w = make<window>(title, 1280, 720, instance_ptr);
        if(!w.has_value()) return w.error();
        RESULT_VERIFY(w->initialize(logi_device_ptr, phys_device_ptr));
        auto new_window = windows.emplace(title, *std::move(w));
        if(!new_window.second) 
            return error::window_already_exists;

        return &(new_window.first->second);
    }


    result<void> application::remove_window(std::string_view title) noexcept {
        if (auto it = windows.find(std::string(title)); it != windows.end()) {
            windows.erase(it);
            return {};
        }
 
        return error::window_not_found;
    }
}

namespace d2d {
    result<window*> application::add_window() noexcept {
        return add_window(name);
    }

    result<void> application::remove_window() noexcept {
        return remove_window(name);
    }
}


namespace d2d {
    bool application::open() const noexcept {
        for (auto& w : windows)
            if(!glfwWindowShouldClose(w.second))
                return true;
        return false;
    }

    result<void> application::render() noexcept {
        for (auto& w : windows) {
            glfwPollEvents();
            if(auto r = w.second.render(); !r.has_value()) 
                return r.error();
        }
        return {};
    }

    result<void> application::join() noexcept {
        __D2D_VULKAN_VERIFY(vkDeviceWaitIdle(*logi_device_ptr));
        return  {};
    }
}

//namespace d2d {
//    std::atomic_int64_t application::instance_count{};
//}