#pragma once
#include <optional>
#include <span>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/memory/buffer.hpp"

namespace d2d {
    class renderable_allocator {
    public:
        constexpr renderable_allocator() noexcept = default;
        constexpr renderable_allocator(logical_device& logi_device, physical_device& phys_device, command_pool& copy_command_pool) noexcept : 
            logi_device_ptr(std::addressof(logi_device)), phys_device_ptr(std::addressof(phys_device)), copy_cmd_pool_ptr(std::addressof(copy_command_pool)) {}

        template<typename InputContainerT>
        result<std::pair<buffer, device_memory<1>>> stage(std::size_t total_buffer_size, InputContainerT&& inputs, std::size_t mem_offset = 0) const noexcept;

        template<std::size_t I, VkFlags BufferUsage, VkMemoryPropertyFlags MemProps, VkMemoryPropertyFlags FallbackMemProps, std::size_t N>
        result<void> alloc_buffer(std::span<buffer, N> buffs, std::size_t total_buffer_size, device_memory<N>& mem) noexcept;

        inline result<void> staging_to_device_local(buffer& device_local_buff, buffer const& staging_buff, std::size_t offset = 0, std::optional<std::size_t> size = std::nullopt) noexcept;
        inline result<void> staging_to_device_local(image& device_local_image, buffer const& staging_buff) noexcept;

        constexpr command_buffer const& copy_command_buffer() const noexcept { return copy_cmd_buffer; }

    private:
        inline result<void> gpu_alloc_begin() noexcept;
        inline result<void> gpu_alloc_end() noexcept;

        template<typename OutputContainerT, typename InputContainerT, std::size_t N>
        result<std::size_t> realloc(OutputContainerT&& output_container, InputContainerT&& input_container, device_memory<N>& new_mem, std::size_t skip_idx, std::size_t starting_offset = 0) const noexcept;

        template<typename OutputContainerT, typename InputContainerT>
        void move(OutputContainerT&& output_container, InputContainerT&& input_container) const noexcept;

    private:
        logical_device* logi_device_ptr;
        physical_device* phys_device_ptr;
        command_pool* copy_cmd_pool_ptr;
        command_buffer copy_cmd_buffer;
    public:
        friend class texture_map;
    };
}

#include "Duo2D/vulkan/memory/renderable_allocator.inl"
