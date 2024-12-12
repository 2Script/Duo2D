#pragma once
#include <cstring>
#include <vulkan/vulkan_core.h>
#include "Duo2D/graphics/pipeline/buffer.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/graphics/pipeline/command_pool.hpp"
#include "Duo2D/graphics/pipeline/render_pass.hpp"
#include "Duo2D/graphics/pipeline/swap_chain.hpp"
#include "Duo2D/traits/renderable_traits.hpp"

namespace d2d { template<std::size_t FiF, impl::RenderableType... Rs> requires (sizeof...(Rs) > 0) struct renderable_buffer; }

namespace d2d {
    struct command_buffer : pipeline_obj_base<VkCommandBuffer> {
        static result<command_buffer> create(logical_device& device, const command_pool& pool) noexcept;
    public:
        result<void> begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept;
        result<void> end() const noexcept;
        result<void> reset() const noexcept;

        template<impl::RenderableType T, std::size_t FiF, impl::RenderableType... Rs>
        result<void> draw(const renderable_buffer<FiF, Rs...>& renderables) const noexcept;
        
        result<void> copy(buffer& dest, const buffer& src, std::size_t size) const noexcept;
    };
}

#include "Duo2D/graphics/pipeline/command_buffer.inl"