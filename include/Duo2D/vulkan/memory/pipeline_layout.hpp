#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipelineLayout);

namespace d2d::vk {
    template<typename T, sl::size_t N, resource_table<N> Resources>
    struct pipeline_layout : vulkan_ptr<VkPipelineLayout, vkDestroyPipelineLayout> {
        static result<pipeline_layout> create(std::shared_ptr<logical_device> device) noexcept;
    };
}

#include "Duo2D/vulkan/memory/pipeline_layout.inl"