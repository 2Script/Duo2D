#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/instance.hpp"


namespace d2d { struct window; }

__D2D_DECLARE_VK_TRAITS_INST(VkSurfaceKHR);

namespace d2d::vk {

    struct surface : vulkan_ptr<VkSurfaceKHR, vkDestroySurfaceKHR> {
        static result<surface> create(const ::d2d::window& w, const instance& i) noexcept;
    };
}
