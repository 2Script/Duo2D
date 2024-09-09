#include "Duo2D/application.hpp"

#include "Duo2D/error.hpp"
#include <GLFW/glfw3.h>


namespace d2d {
    result<void> application::add_window(std::string_view title) noexcept {
        window w{};

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporary
        w.handle = glfwCreateWindow(1280, 720, title.data(), nullptr, nullptr);

        __D2D_GLFW_VERIFY(w.handle);

        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(vulkan_instance, w.handle, nullptr, &w.surface));

        if(!windows.emplace(title, w).second) 
            return error::window_already_exists;
        return result<void>{std::in_place_type<void>};
    }
}

namespace d2d {
    result<void> application::remove_window(std::string_view title) noexcept {
        if (auto it = windows.find(std::string(title)); it != windows.end()) {
            vkDestroySurfaceKHR(vulkan_instance, it->second.surface, nullptr);
            glfwDestroyWindow(it->second.handle);
            windows.erase(it);
            return result<void>{std::in_place_type<void>};
        }
 
        return error::window_not_found;
    }
}