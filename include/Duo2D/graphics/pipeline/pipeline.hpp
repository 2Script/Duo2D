#pragma once
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_layout.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include "Duo2D/graphics/prim/renderable_traits.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipeline);

namespace d2d {
    template<impl::RenderableType T>
    struct pipeline : pipeline_obj<VkPipeline, vkDestroyPipeline> {
        static result<pipeline> create(logical_device& device, render_pass& associated_render_pass, pipeline_layout& layout) noexcept;
    };
}

#include "Duo2D/graphics/pipeline/pipeline.inl"