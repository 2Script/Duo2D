#pragma once
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"

namespace d2d {
    struct window;

    struct command_buffer : pipeline_obj_base<VkCommandBuffer> {
        static result<command_buffer> create(logical_device& deivce, command_pool& pool) noexcept;
    public:
        result<void> record(const window& w, std::uint32_t image_index) const noexcept;
        result<void> reset() const noexcept;
    };
}
