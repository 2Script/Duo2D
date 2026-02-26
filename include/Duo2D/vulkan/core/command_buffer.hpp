#pragma once
#include "Duo2D/vulkan/core/command_buffer.fwd.hpp"

#include <cstddef>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>
#include <frozen/unordered_map.h>

#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/vulkan/memory/device_allocation.fwd.hpp"
#include "Duo2D/vulkan/memory/device_allocation_segment.hpp"
#include "Duo2D/vulkan/memory/image.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include "Duo2D/vulkan/sync/semaphore.hpp"


namespace d2d::vk {
	template<sl::size_t N>
    struct command_buffer : vulkan_ptr_base<VkCommandBuffer> {
        static result<command_buffer> create(std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> pool) noexcept;
    public:
        result<void> begin(bool one_time = false) const noexcept;
        result<void> end() const noexcept;

        result<void> reset() const noexcept;
        result<void> submit(command_family_t family, std::span<const semaphore_submit_info> wait_semaphore_infos = {}, std::span<const semaphore_submit_info> signal_semaphore_infos = {}, VkFence out_fence = VK_NULL_HANDLE) const noexcept;
        result<void> wait(command_family_t family) const noexcept;
        void free() const noexcept;
		
		
		template<typename T, sl::index_t I, typename Derived, resource_table<N> Resources>
		void bind_buffer(device_allocation_segment<I, Derived> const& buff, pipeline_layout<shader_stage::all_graphics, T, N, Resources> const& layout, std::size_t& bind_index) const noexcept;
		template<typename T, sl::index_t I, typename Derived, resource_table<N> Resources>
		void bind_buffer(device_allocation_segment<I, Derived> const& buff, pipeline_layout<shader_stage::compute, T, N, Resources> const& layout, std::size_t& bind_index) const noexcept;
    	template<bind_point_t BindPoint, typename T, resource_table<N> Resources>
		void bind_pipeline(pipeline<BindPoint, T, N, Resources> const& p) const noexcept;

	public:
        void begin_draw(std::span<const VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo const& depth_attachment, rect<std::uint32_t> render_area_bounds, rect<float> viewport_bounds, rect<std::uint32_t> scissor_bounds) const noexcept;
        void end_draw() const noexcept;

    public:
		template<typename T>
        void draw(sl::uoffset_t draw_offset = 0, sl::uoffset_t count_offset = 0) const noexcept;
		template<typename T>
        void dispatch(sl::uoffset_t offset = 0) const noexcept;
        
    public:
        void pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept;
        void image_transition(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count = 1) const noexcept;
    public:
		template<sl::index_t I, sl::index_t J, typename Derived>
        void copy(device_allocation_segment<I, Derived>& dst, device_allocation_segment<J, Derived> const& src, std::span<const VkBufferCopy> copy_regions) const noexcept;
		template<sl::index_t I, sl::index_t J, typename Derived>
        void copy(device_allocation_segment<I, Derived>& dst, device_allocation_segment<J, Derived> const& src, std::size_t size, sl::uoffset_t dst_offset = 0, sl::uoffset_t src_offset = 0) const noexcept;
    private:
        constexpr static frozen::unordered_map<VkImageLayout, std::pair<VkPipelineStageFlagBits2, VkAccessFlagBits2>, 4> image_barrier_map {
            {VK_IMAGE_LAYOUT_UNDEFINED,                {VK_PIPELINE_STAGE_2_NONE,                VK_ACCESS_2_NONE}},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_WRITE_BIT}},
        };


    private:
        std::shared_ptr<logical_device> logi_device_ptr;
        std::shared_ptr<physical_device> phys_device_ptr;
        std::shared_ptr<command_pool>   cmd_pool_ptr;

		//TODO: just make the command buffer functions not const
		mutable sl::array<N, VkBuffer> draw_buff_refs;
		mutable sl::array<N, sl::size_t> draw_buff_sizes;
		mutable sl::size_t draw_buff_ref_count;
		mutable sl::array<N, VkBuffer> draw_count_buff_refs;
		mutable sl::size_t draw_count_buff_ref_count;
		mutable sl::array<N, VkBuffer> dispatch_buff_refs;
		mutable sl::size_t dispatch_buff_ref_count;
    };
}

#include "Duo2D/vulkan/core/command_buffer.inl"