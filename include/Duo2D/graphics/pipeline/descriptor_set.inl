#include "Duo2D/graphics/pipeline/descriptor_set.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"
#include <algorithm>
#include <utility>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::uint32_t FramesInFlight>
    result<descriptor_set<FramesInFlight>> descriptor_set<FramesInFlight>::create(logical_device& device, descriptor_pool<FramesInFlight>& pool, descriptor_set_layout& layout, std::array<buffer, FramesInFlight>& uniform_buffers) noexcept {
        descriptor_set ret{};

        std::array<VkDescriptorSetLayout, FramesInFlight> layouts;
        layouts.fill(static_cast<VkDescriptorSetLayout>(layout));

        VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pool,
            .descriptorSetCount = FramesInFlight,
            .pSetLayouts = layouts.data(),
        };

        __D2D_VULKAN_VERIFY(vkAllocateDescriptorSets(device, &alloc_info, ret.sets.data()));

        for (size_t i = 0; i < FramesInFlight; i++) {
            VkDescriptorBufferInfo buffer_info{
                .buffer = uniform_buffers[i],
                .offset = 0,
                .range = sizeof(vertex2::uniform_type),
            };

            VkWriteDescriptorSet descriptor_write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = ret.sets[i],
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