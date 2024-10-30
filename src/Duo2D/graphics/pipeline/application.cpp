#include "Duo2D/graphics/pipeline/application.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>

#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"


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
            VK_API_VERSION_1_0        //API Version
        };
        
        // Initialize GLFW
        if(int code; !glfw_init && !glfwInit() && (code = glfwGetError(nullptr)))
            return __D2D_GLFW_ERR;
        glfw_init = true;

        // Check for vulkan support
        if(!glfwVulkanSupported()) 
            return error::vulkan_not_supported;

        // Create instance
        application ret{};
        __D2D_TRY_MAKE(ret.vk_instance, make<instance>(app_info), v)
        ret.name = name;

        return ret;
    }
}


namespace d2d {
    result<std::set<physical_device>> application::devices() const noexcept {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

        if(!device_count)
            return error::no_vulkan_devices;

        std::vector<VkPhysicalDevice> devices(device_count);
        __D2D_VULKAN_VERIFY(vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data()));

        //Create dummy window
        __D2D_TRY_MAKE(window dummy, make<window>("dummy", 1280, 720, vk_instance), var_name);


        std::set<physical_device> ret{};
        for(VkPhysicalDevice device_handle : devices) {
            result<physical_device> d = make<physical_device>(device_handle, dummy);
            if(!d.has_value()) return d.error();
            ret.insert(*std::move(d));
        }

        return ret;
    }


    result<void> application::initialize_device() noexcept {
        //Create logical device
        __D2D_TRY_MAKE(logi_device, make<logical_device>(phys_device), ld);

        //Create queues
        for(std::size_t i = 0; i < queue_family::num_families; ++i)
            vkGetDeviceQueue(logi_device, *(phys_device.queue_family_idxs[i]), 0, &queues[i]);

        return result<void>{std::in_place_type<void>};
    }
}


namespace d2d {
    result<void> application::add_window(std::string_view title) noexcept {
        if(!logi_device)
            return error::device_not_initialized;

        
        result<window> w = make<window>(title, 1280, 720, vk_instance);
        if(!w.has_value()) return w.error();
        w->initialize_swap(logi_device, phys_device);
        if(!windows.emplace(title, *std::move(w)).second) 
            return error::window_already_exists;

        return result<void>{std::in_place_type<void>};
    }


    result<void> application::remove_window(std::string_view title) noexcept {
        if (auto it = windows.find(std::string(title)); it != windows.end()) {
            windows.erase(it);
            return result<void>{std::in_place_type<void>};
        }
 
        return error::window_not_found;
    }
}

namespace d2d {
    result<void> application::add_window() noexcept {
        return add_window(name);
    }

    result<void> application::remove_window() noexcept {
        return remove_window(name);
    }
}