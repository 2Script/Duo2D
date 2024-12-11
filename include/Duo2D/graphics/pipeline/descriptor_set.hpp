#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/descriptor_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/prim/renderable_traits.hpp"


namespace d2d {
    template<std::size_t FramesInFlight>
    struct descriptor_set : std::array<VkDescriptorSet, FramesInFlight> {
        template<std::size_t DC>
        static result<descriptor_set> create(logical_device& device, descriptor_pool<FramesInFlight, DC>& pool, descriptor_set_layout& layout, const buffer& uniform_buffer, std::size_t data_size, std::size_t buffer_offset) noexcept;
    };
}

#include "Duo2D/graphics/pipeline/descriptor_set.inl"