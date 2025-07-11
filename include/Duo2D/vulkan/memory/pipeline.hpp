#pragma once
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/traits/directly_renderable.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipeline);

namespace d2d::vk {
    template<::d2d::impl::directly_renderable T>
    struct pipeline : vulkan_ptr<VkPipeline, vkDestroyPipeline> {
        static result<pipeline> create(std::shared_ptr<logical_device> device, render_pass& associated_render_pass, pipeline_layout<T>& layout) noexcept;
    };
}

#include "Duo2D/vulkan/memory/pipeline.inl"