#pragma once
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/traits/renderable_traits.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipeline);

namespace d2d {
    template<impl::RenderableType T>
    struct pipeline : vulkan_ptr<VkPipeline, vkDestroyPipeline> {
        static result<pipeline> create(logical_device& device, render_pass& associated_render_pass, pipeline_layout<T>& layout) noexcept;
    };
}

#include "Duo2D/vulkan/memory/pipeline.inl"