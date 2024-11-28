#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    result<descriptor_set_layout> descriptor_set_layout::create(logical_device& device) noexcept {
        descriptor_set_layout ret{};
        ret.dependent_handle = device;

        VkDescriptorSetLayoutBinding descriptor_layout_binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };
        
        VkDescriptorSetLayoutCreateInfo descriptor_layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &descriptor_layout_binding,
        };
        __D2D_VULKAN_VERIFY(vkCreateDescriptorSetLayout(device, &descriptor_layout_info, nullptr, &ret.handle));
        return ret;
    }
}