#include "Duo2D/vulkan/display/surface.hpp"
#include "Duo2D/core/window.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace d2d::vk {
    result<surface> surface::create(::d2d::window const& w, std::shared_ptr<instance> i) noexcept {
        surface ret{};
        ret.dependent_handle = i;
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(*i, w, nullptr, &ret.handle));
        return ret;
    }
}