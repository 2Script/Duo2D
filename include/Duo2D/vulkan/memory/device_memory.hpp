#pragma once
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);

namespace d2d {
    template<std::size_t N>
    struct device_memory : vulkan_ptr<VkDeviceMemory, vkFreeMemory> {
        static result<device_memory> create(logical_device& logi_device, physical_device& phys_device, const std::array<buffer, N>& associated_buffers, VkMemoryPropertyFlags properties) noexcept;
        result<void> bind(logical_device& device, std::array<buffer, N>& associated_buffers, std::size_t buff_idx, command_pool& cmd_pool) const noexcept;

        constexpr const std::array<VkMemoryRequirements, N>& requirements() const noexcept { return mem_reqs; }
    
    private:
        std::array<VkMemoryRequirements, N> mem_reqs = {};
    };
}

#include "Duo2D/vulkan/memory/device_memory.inl"