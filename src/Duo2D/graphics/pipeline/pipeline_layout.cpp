#include "Duo2D/graphics/pipeline/pipeline_layout.hpp"

namespace d2d {
    result<pipeline_layout> pipeline_layout::create(logical_device& device) noexcept {
        pipeline_layout ret{};
        ret.dependent_handle = device;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0,
        };
        __D2D_VULKAN_VERIFY(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &ret.handle));

        return ret;
    }
}
