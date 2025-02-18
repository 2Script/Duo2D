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
        static result<device_memory> create(logical_device& logi_device, physical_device& phys_device, std::span<buffer, N> associated_buffers, VkMemoryPropertyFlags properties) noexcept;

        void unmap(logical_device& device) const noexcept;
        result<void*> map(logical_device& device, std::size_t size, std::size_t offset = 0) const noexcept;
        result<void> bind(logical_device& device, buffer& buff, std::size_t offset) const noexcept;

        constexpr const std::array<VkMemoryRequirements, N>& requirements() const noexcept { return mem_reqs; }
    
    public:
        constexpr device_memory() noexcept = default;
        ~device_memory() noexcept = default;
        constexpr device_memory(device_memory&& other) noexcept;
        constexpr device_memory& operator=(device_memory&& other) noexcept;
        device_memory(const device_memory& other) = delete;
        device_memory& operator=(const device_memory& other) = delete;

    private:
        std::array<VkMemoryRequirements, N> mem_reqs = {};
        mutable bool mapped = false;
    };
}

#include "Duo2D/vulkan/memory/device_memory.inl"