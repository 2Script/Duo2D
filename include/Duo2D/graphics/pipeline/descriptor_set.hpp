#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/descriptor_pool.hpp"
#include "Duo2D/graphics/pipeline/descriptor_set_layout.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"


namespace d2d {
    template<std::uint32_t FramesInFlight>
    struct descriptor_set {
        static result<descriptor_set> create(logical_device& device, descriptor_pool<FramesInFlight>& pool, descriptor_set_layout& layout, std::array<buffer, FramesInFlight>& uniform_buffers) noexcept;
    
        constexpr VkDescriptorSet const* data() const noexcept { return sets.data(); }
    private:
        std::array<VkDescriptorSet, FramesInFlight> sets;
    };
}

#include "Duo2D/graphics/pipeline/descriptor_set.inl"