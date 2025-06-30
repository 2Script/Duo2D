#include "Duo2D/vulkan/display/surface.hpp"

namespace d2d::vk {
    result<surface> surface::create(GLFWwindow* w, std::shared_ptr<instance> i) noexcept {
        surface ret{};
        ret.dependent_handle = i;
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(*i, w, nullptr, &ret.handle));
        return ret;
    }
}