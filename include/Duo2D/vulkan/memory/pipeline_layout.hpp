#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/descriptor_set_layout.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipelineLayout);

namespace d2d {
    template<impl::renderable_like T>
    struct pipeline_layout : vulkan_ptr<VkPipelineLayout, vkDestroyPipelineLayout> {
        static result<pipeline_layout> create(logical_device& device, descriptor_set_layout& set_layout) noexcept;
        static result<pipeline_layout> create(logical_device& device) noexcept;
    };
}

#include "Duo2D/vulkan/memory/pipeline_layout.inl"