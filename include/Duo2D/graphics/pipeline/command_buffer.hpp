#pragma once
#include <cstring>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/shader_buffer.hpp"

namespace d2d {
    struct window;

    struct command_buffer : pipeline_obj_base<VkCommandBuffer> {
        static result<command_buffer> create(logical_device& deivce, command_pool& pool) noexcept;
    public:
        result<void> record(const window& w, std::vector<VkBuffer>& vertex_buffers, shader_buffer& index_buffer, std::vector<std::size_t>& offsets, std::size_t index_count, std::uint32_t image_index) const noexcept;
        result<void> reset() const noexcept;
        
        result<void> copy(buffer& dest, const buffer& src, std::size_t size) const noexcept;
    };
}
