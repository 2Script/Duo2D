#pragma once
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"

namespace d2d::vk {
    template<::d2d::impl::directly_renderable T>
    result<pipeline_layout<T>> pipeline_layout<T>::create(std::shared_ptr<logical_device> device, descriptor_set_layout& set_layout) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = device;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &set_layout,
        };
        if constexpr (renderable_constraints<T>::has_push_constants) {
            constexpr static std::array ranges = T::push_constant_ranges();
            pipeline_layout_create_info.pushConstantRangeCount = ranges.size();
            pipeline_layout_create_info.pPushConstantRanges = ranges.data();
        }

        __D2D_VULKAN_VERIFY(vkCreatePipelineLayout(*device, &pipeline_layout_create_info, nullptr, &ret.handle));

        return ret;
    }

    template<::d2d::impl::directly_renderable T>
    result<pipeline_layout<T>> pipeline_layout<T>::create(std::shared_ptr<logical_device> device) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = device;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
        };
        if constexpr (renderable_constraints<T>::has_push_constants) {
            constexpr static std::array ranges = T::push_constant_ranges();
            pipeline_layout_create_info.pushConstantRangeCount = ranges.size();
            pipeline_layout_create_info.pPushConstantRanges = ranges.data();
        }

        __D2D_VULKAN_VERIFY(vkCreatePipelineLayout(*device, &pipeline_layout_create_info, nullptr, &ret.handle));

        return ret;
    }
}
