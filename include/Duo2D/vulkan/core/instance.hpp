#pragma once
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/traits/vk_traits.hpp"
#include <string_view>
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS(VkInstance);

namespace d2d::vk {
    struct instance : vulkan_ptr<VkInstance, vkDestroyInstance> {
        static result<instance> create(VkApplicationInfo& app_info) noexcept;

    private:
        constexpr static std::array<const std::string_view, 1> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };
}
