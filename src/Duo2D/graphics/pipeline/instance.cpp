#include "Duo2D/graphics/pipeline/instance.hpp"

namespace d2d {
    result<instance> instance::create(VkApplicationInfo& app_info) noexcept {
        // Get needed extensions
        uint32_t glfw_ext_cnt = 0;
        const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
        __D2D_GLFW_VERIFY(glfw_exts);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_ext_cnt;
        create_info.ppEnabledExtensionNames = glfw_exts;
        create_info.enabledLayerCount = 0;
        
        instance ret{};
        __D2D_VULKAN_VERIFY(vkCreateInstance(&create_info, nullptr, &ret.handle));
        return ret;
    }
}