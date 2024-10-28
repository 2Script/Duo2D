#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/instance.hpp"


__D2D_DECLARE_VK_TRAITS_INST(VkSurfaceKHR);

namespace d2d {
    struct window;

    struct surface : pipeline_obj<VkSurfaceKHR, vkDestroySurfaceKHR> {
        static result<surface> create(const window& w, const instance& i) noexcept;
    };
}
