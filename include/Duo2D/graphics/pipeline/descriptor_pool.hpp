#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorPool);

namespace d2d {
    template<std::uint32_t FramesInFlight>
    struct descriptor_pool : pipeline_obj<VkDescriptorPool, vkDestroyDescriptorPool> {
        static result<descriptor_pool> create(logical_device& device) noexcept;
    };
}

#include "Duo2D/graphics/pipeline/descriptor_pool.inl"