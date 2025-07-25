#pragma once
#include <cstddef>
#include <memory>
#include <span>
#include <vulkan/vulkan.h>
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/image.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/traits/directly_renderable.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE_AUX(VkCommandBuffer, command_pool);

namespace d2d::vk { template<std::size_t FramesInFlight, typename> class renderable_tuple; }


namespace d2d::vk {
    struct command_buffer : vulkan_ptr_base<VkCommandBuffer> {
        static result<command_buffer> create(std::shared_ptr<logical_device> device, std::shared_ptr<command_pool> pool) noexcept;
    public:
        result<void> render_begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept;
        result<void> render_end() const noexcept;
        result<void> reset() const noexcept;

        template<::d2d::impl::directly_renderable T, std::size_t FiF, ::d2d::impl::directly_renderable... Rs> 
        result<void> draw(const renderable_tuple<FiF, std::tuple<Rs...>>& renderables) const noexcept; 
        
        result<void> generic_begin() const noexcept;
        void copy_alike(buffer& dest, const buffer& src, std::size_t size, std::size_t offset = 0) const noexcept;
        void copy_alike(image& dest, image& src, extent2 size) const noexcept;
        void copy_buffer_to_image(image& dest, const buffer& src, extent2 image_size, std::size_t buffer_offset = 0, std::uint32_t array_idx = 0, std::uint32_t array_count = 1) const noexcept;
        void copy_buffer_to_image(image& dest, const buffer& src, std::span<const VkBufferImageCopy> copy_regions) const noexcept;
        void transition_image(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count = 1) const noexcept;
        result<void> generic_end() const noexcept;

    private:
        std::shared_ptr<logical_device> logi_device_ptr;
        std::shared_ptr<command_pool>   cmd_pool_ptr;
    };
}

#include "Duo2D/vulkan/core/command_buffer.inl"