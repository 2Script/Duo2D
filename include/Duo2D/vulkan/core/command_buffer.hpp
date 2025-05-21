#pragma once
#include <cstring>
#include <optional>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE_AUX(VkCommandBuffer, VkCommandPool);

namespace d2d { template<std::size_t FiF, impl::RenderableType... Rs> /*requires (sizeof...(Rs) > 0)*/ struct renderable_buffer; }


namespace d2d {
    struct command_buffer : vulkan_ptr_base<VkCommandBuffer> {
        static result<command_buffer> create(logical_device& device, const command_pool& pool) noexcept;
    public:
        result<void> begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept;
        result<void> end() const noexcept;
        result<void> reset() const noexcept;

        template<impl::RenderableType T, std::size_t FiF, impl::RenderableType... Rs>
        result<void> draw(const renderable_buffer<FiF, Rs...>& renderables) const noexcept; 
        
        result<void> copy_begin() const noexcept;
        void copy_generic(buffer& dest, const buffer& src, std::size_t size) const noexcept;
        void copy_image(buffer& dest, const buffer& src, extent2 size) const noexcept;
        result<void> copy_end(logical_device& device, const command_pool& pool) const noexcept;
    };
}

#include "Duo2D/vulkan/core/command_buffer.inl"