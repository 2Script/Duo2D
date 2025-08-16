#pragma once
#include <cstddef>
#include <frozen/unordered_map.h>
#include <memory>
#include <span>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/memory/buffer.hpp"
#include "Duo2D/vulkan/memory/image.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/display/swap_chain.hpp"
#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"

__D2D_DECLARE_VK_TRAITS_DEVICE_AUX(VkCommandBuffer, command_pool);

namespace d2d::vk { template<std::size_t FramesInFlight, typename> class renderable_tuple; }


namespace d2d::vk {
    struct command_buffer : vulkan_ptr_base<VkCommandBuffer> {
        static result<command_buffer> create(std::shared_ptr<logical_device> device, std::shared_ptr<command_pool> pool) noexcept;
    public:
        result<void> begin(bool one_time = true) const noexcept;
        result<void> end() const noexcept;

        result<void> reset() const noexcept;
        result<void> submit(queue_family::id family, std::span<const semaphore_submit_info> wait_semaphore_infos = {}, std::span<const semaphore_submit_info> signal_semaphore_infos = {}, VkFence out_fence = VK_NULL_HANDLE) const noexcept;
        result<void> wait(queue_family::id family) const noexcept;
        void free() const noexcept;

        void render_begin(const swap_chain& window_swap_chain, const render_pass& window_render_pass, std::uint32_t image_index) const noexcept;
        void render_end() const noexcept;

    public:
        template<::d2d::impl::directly_renderable T, std::size_t FiF, typename TupleT> 
        result<void> draw(renderable_tuple<FiF, TupleT> const& renderables) const noexcept; 
        //TODO?
        //template<typename T, std::size_t FiF, typename TupleT> requires (!::d2d::impl::directly_renderable<T>)
        //result<void> draw(renderable_tuple<FiF, TupleT> const&) const noexcept { return {}; }
        
    public:
        void pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept;
        void image_transition(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count = 1) const noexcept;
    public:
        void copy_alike(buffer& dst, const buffer& src, std::size_t size, std::size_t offset = 0) const noexcept;
        void copy_alike(image& dst, image& src, extent2 size) const noexcept;

        void copy_buffer_to_image(image& dst, const buffer& src, VkImageLayout dest_final_layout, extent2 image_size, std::size_t buffer_offset = 0, std::uint32_t image_idx = 0, std::uint32_t image_count = 1) const noexcept;
        void copy_buffer_to_image(image& dst, const buffer& src, VkImageLayout dest_final_layout, std::span<const VkBufferImageCopy> copy_regions) const noexcept;
    private:
        constexpr static frozen::unordered_map<VkImageLayout, std::pair<VkPipelineStageFlagBits2, VkAccessFlagBits2>, 4> image_barrier_map {
            {VK_IMAGE_LAYOUT_UNDEFINED,                {VK_PIPELINE_STAGE_2_NONE,                VK_ACCESS_2_NONE}},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_WRITE_BIT}},
        };


    private:
        std::shared_ptr<logical_device> logi_device_ptr;
        std::shared_ptr<command_pool>   cmd_pool_ptr;
    };
}

#include "Duo2D/vulkan/core/command_buffer.inl"