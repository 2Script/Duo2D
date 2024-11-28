#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/device_memory.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/physical_device.hpp"

namespace d2d {
    enum class buffer_type : std::uint8_t {
        vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    };
}

namespace d2d {
    struct shader_buffer {
        static result<shader_buffer> create(logical_device& logi_device, physical_device& phys_device, command_pool& pool, void const* data, std::size_t data_size, buffer_type type) noexcept;

        constexpr explicit operator VkBuffer() const noexcept { return _buffer; }
        constexpr explicit operator VkDeviceMemory() const noexcept { return _buffer_memory; }
    private:
        buffer _buffer;
        device_memory _buffer_memory;
    };
}
