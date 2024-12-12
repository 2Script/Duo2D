#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/vulkan/core/window.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace d2d {
    result<surface> surface::create(window const& w, instance const& i) noexcept {
        surface ret{};
        ret.dependent_handle = i;
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(i, w, nullptr, &ret.handle));
        return ret;
    }
}