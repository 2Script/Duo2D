#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan.h>

#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/vulkan/core/instance.hpp"


__D2D_DECLARE_VK_TRAITS_INST(VkSurfaceKHR);

namespace acma::vk {
    struct surface : vulkan_ptr_base<VkSurfaceKHR> {
		constexpr surface() noexcept = default;
        static result<surface> create(GLFWwindow* w) noexcept;

	public:
		~surface() noexcept;
	public:
	    surface(surface&& other) noexcept;
        surface& operator=(surface&& other) noexcept;

        constexpr surface(const surface& other) = delete;
        constexpr surface& operator=(const surface& other) = delete;
    };
}
