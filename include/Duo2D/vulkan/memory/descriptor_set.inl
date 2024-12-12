#pragma once
#include "Duo2D/vulkan/memory/descriptor_set.hpp"
#include "Duo2D/error.hpp"
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t FiF>
    template<std::size_t DC>
    result<descriptor_set<FiF>> descriptor_set<FiF>::create(logical_device& device, descriptor_pool<FiF, DC>& pool, descriptor_set_layout& layout, const buffer& uniform_buffer, std::size_t data_size, std::size_t buffer_offset) noexcept {
        descriptor_set ret{};

        std::array<VkDescriptorSetLayout, FiF> layouts;
        layouts.fill(static_cast<VkDescriptorSetLayout>(layout));

        VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pool,
            .descriptorSetCount = FiF,
            .pSetLayouts = layouts.data(),
        };

        __D2D_VULKAN_VERIFY(vkAllocateDescriptorSets(device, &alloc_info, ret.data()));

        for (size_t i = 0; i < FiF; i++) {
            VkDescriptorBufferInfo buffer_info{
                .buffer = uniform_buffer,
                .offset = buffer_offset + (i * data_size),
                .range = data_size,
            };

            VkWriteDescriptorSet descriptor_write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = ret[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffer_info,
            };

            vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
        }

        return ret;
    }
}