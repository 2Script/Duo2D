#pragma once
#include <span>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/prim/texture.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/memory/texture_map_base.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);

namespace d2d {
    struct device_memory_base : public vulkan_ptr<VkDeviceMemory, vkFreeMemory> {
    public:
        inline void unmap(logical_device& device) const noexcept;
        inline result<void*> map(logical_device& device, std::size_t size, std::size_t offset = 0) const noexcept;
    protected:
        template<typename MemReqsContainer>
        inline result<void> allocate(logical_device& logi_device, std::optional<std::uint32_t> mem_type_idx, MemReqsContainer&& mem_reqs) noexcept;
    public:
        /*
        constexpr device_memory_base() noexcept = default;
        ~device_memory_base() noexcept = default;
        constexpr device_memory_base(device_memory_base&& other) noexcept;
        constexpr device_memory_base& operator=(device_memory_base&& other) noexcept;
        device_memory_base(const device_memory_base& other) = delete;
        device_memory_base& operator=(const device_memory_base& other) = delete;
        */

    protected:
        mutable bool mapped = false;
    };
}

namespace d2d {
    template<std::size_t N>
    struct device_memory : public device_memory_base {
        static result<device_memory> create(logical_device& logi_device, physical_device& phys_device, std::span<buffer, N> associated_buffers, VkMemoryPropertyFlags properties) noexcept;

        result<void> bind(logical_device& device, buffer& buff, std::size_t offset) const noexcept;

        constexpr const std::array<VkMemoryRequirements, N>& requirements() const noexcept { return mem_reqs; }
    
    public:
        /*
        constexpr device_memory() noexcept = default;
        ~device_memory() noexcept = default;
        constexpr device_memory(device_memory&& other) noexcept;
        constexpr device_memory& operator=(device_memory&& other) noexcept;
        device_memory(const device_memory& other) = delete;
        device_memory& operator=(const device_memory& other) = delete;
        */

    private:
        std::array<VkMemoryRequirements, N> mem_reqs = {};
    };


    template<>
    struct device_memory<std::dynamic_extent> : public device_memory_base {
        inline static result<device_memory> create(logical_device& logi_device, physical_device& phys_device, texture_map_base& textures, buffer& texture_size_buffer, VkMemoryPropertyFlags properties) noexcept;

        inline result<void> bind(logical_device& device, buffer& buff, std::size_t offset) const noexcept;
        inline result<void> bind(logical_device& device, image& img, std::size_t offset) const noexcept;

        constexpr const std::vector<VkMemoryRequirements>& requirements() const noexcept { return mem_reqs; }
    
    
    public:
        /*
        constexpr device_memory() noexcept = default;
        ~device_memory() noexcept = default;
        constexpr device_memory(device_memory&& other) noexcept;
        constexpr device_memory& operator=(device_memory&& other) noexcept;
        device_memory(const device_memory& other) = delete;
        device_memory& operator=(const device_memory& other) = delete;\
        */

    private:
        std::vector<VkMemoryRequirements> mem_reqs = {};
    };
}

#include "Duo2D/vulkan/memory/device_memory.inl"