#include "Duo2D/application.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>

#include "Duo2D/error.hpp"
#include "Duo2D/hardware/device/device_info.hpp"


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

        // Get needed extensions
        uint32_t glfw_ext_cnt = 0;
        const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
        __D2D_GLFW_VERIFY(glfw_exts);


        // Set enabled extensions
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_ext_cnt;
        create_info.ppEnabledExtensionNames = glfw_exts;
        create_info.enabledLayerCount = 0;

        // Create instance
        application ret{};
        __D2D_VULKAN_VERIFY(vkCreateInstance(&create_info, nullptr, &ret.vulkan_instance));
        ret.name = name;

        return ret;
    }


    result<application> make_application(std::string_view name) noexcept {
        return application::create(name);
    }
}


namespace d2d {
    application::~application() noexcept {
        if(vulkan_instance == VK_NULL_HANDLE) return;
        

        for(auto& w : windows) {
            for(auto p : w.second.image_views)
                vkDestroyImageView(logical_device, p, nullptr);
            vkDestroySwapchainKHR(logical_device, w.second.swap_chain, nullptr);
            vkDestroySurfaceKHR(vulkan_instance, w.second.surface, nullptr);
            glfwDestroyWindow(w.second.handle);
        }

        if(logical_device)
            vkDestroyDevice(logical_device, nullptr);

        vkDestroyInstance(vulkan_instance, nullptr);
        glfwTerminate();
        glfw_init = false;
    }
    

    application::application(application&& other) noexcept : 
        vulkan_instance(other.vulkan_instance),
        physical_device(other.physical_device),
        logical_device(other.logical_device),
        device_format(std::move(other.device_format)),
        device_present_mode(other.device_present_mode),
        name(std::move(other.name)),
        queues(other.queues),
        windows(std::move(other.windows)) {
        other.physical_device = device_info{};
        other.vulkan_instance = VK_NULL_HANDLE;
        other.logical_device  = VK_NULL_HANDLE;
    }
    
    application& application::operator=(application&& other) noexcept {
        vulkan_instance     = other.vulkan_instance;
        physical_device     = other.physical_device;
        logical_device      = other.logical_device;
        device_format       = std::move(other.device_format);
        device_present_mode = other.device_present_mode;
        name                = std::move(other.name);
        queues              = other.queues;
        windows             = std::move(other.windows);
        other.physical_device = device_info{};
        other.vulkan_instance = VK_NULL_HANDLE;
        other.logical_device  = VK_NULL_HANDLE;
        return *this;
    }
}
