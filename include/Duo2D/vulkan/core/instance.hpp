#pragma once
#include <string_view>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/traits/vk_traits.hpp"


__D2D_DECLARE_VK_TRAITS(VkInstance);

namespace d2d::vk {
    struct instance : vulkan_ptr<VkInstance, vkDestroyInstance> {
        static result<instance> create(VkApplicationInfo& app_info, std::span<char const* const> instance_extension_names) noexcept;

    private:
        constexpr static std::array<const std::string_view, 1> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };
}
