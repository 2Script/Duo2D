#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/instance.hpp"


__D2D_DECLARE_VK_TRAITS_INST(VkSurfaceKHR);

namespace d2d::vk {
    struct surface : vulkan_ptr<VkSurfaceKHR, vkDestroySurfaceKHR> {
        static result<surface> create(GLFWwindow* w, std::shared_ptr<instance> i) noexcept;
    };
}
