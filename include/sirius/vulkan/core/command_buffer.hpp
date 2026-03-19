#pragma once
#include "sirius/vulkan/core/command_buffer.fwd.hpp"

#include <cstddef>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>
#include <frozen/unordered_map.h>

#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/render_process.fwd.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"
#include "sirius/vulkan/memory/device_allocation_segment.hpp"
#include "sirius/vulkan/memory/asset_heap_allocation.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/arith/rect.hpp"
#include "sirius/vulkan/memory/pipeline.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"


namespace acma::vk {
    struct command_buffer : vulkan_ptr_base<VkCommandBuffer> {
	private:
		template<typename Seq, sl::size_t N, buffer_config_table<N> BufferConfigs, buffer_usage_policy_flags_t BufferUsage>
		using filtered_by_usage_sequence_type = sl::filtered_sequence_t<Seq, []<buffer_key_t K>(buffer_key_constant_type<K>){
			return BufferConfigs[K].usage & BufferUsage;
		}>;

		template<typename T, sl::size_t N, buffer_config_table<N> BufferConfigs>
		using     draw_command_buffers_sequence_type = filtered_by_usage_sequence_type<decltype(T::buffers), N, BufferConfigs, buffer_usage_policy::draw_commands>;
		template<typename T, sl::size_t N, buffer_config_table<N> BufferConfigs>
		using       draw_count_buffers_sequence_type = filtered_by_usage_sequence_type<decltype(T::buffers), N, BufferConfigs, buffer_usage_policy::draw_count>;
		template<typename T, sl::size_t N, buffer_config_table<N> BufferConfigs>
		using dispatch_command_buffers_sequence_type = filtered_by_usage_sequence_type<decltype(T::buffers), N, BufferConfigs, buffer_usage_policy::dispatch_commands>;

		template<typename T, sl::size_t N, buffer_config_table<N> BufferConfigs>
		constexpr static sl::size_t draw_command_count = std::min(draw_command_buffers_sequence_type<T, N, BufferConfigs>::size(), draw_count_buffers_sequence_type<T, N, BufferConfigs>::size());
		template<typename T, sl::size_t N, buffer_config_table<N> BufferConfigs>
		constexpr static sl::size_t dispatch_command_count = dispatch_command_buffers_sequence_type<T, N, BufferConfigs>::size();

	public:
        inline static result<command_buffer> create(std::shared_ptr<logical_device> logi_device, physical_device* phys_device, std::shared_ptr<command_pool> pool) noexcept;
    public:
    	inline result<void> begin(bool one_time = false) const noexcept;
    	inline result<void> end() const noexcept;

    	inline result<void> reset() const noexcept;
    	inline result<void> submit(command_family_t family, std::span<const semaphore_submit_info> wait_semaphore_infos = {}, std::span<const semaphore_submit_info> signal_semaphore_infos = {}, VkFence out_fence = VK_NULL_HANDLE) const noexcept;
    	inline result<void> wait(command_family_t family) const noexcept;
    	inline void free() const noexcept;
		
		
		template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename T, auto AssetHeapConfigs, typename RenderProcessT>
		void bind_buffer(device_allocation_segment<I, N, BufferConfigs, RenderProcessT> const& buff, pipeline_layout<shader_stage::all_graphics, T, BufferConfigs, AssetHeapConfigs> const& layout) const noexcept;
		template<sl::index_t I, sl::size_t N, buffer_config_table<N> BufferConfigs, typename T, auto AssetHeapConfigs, typename RenderProcessT>
		void bind_buffer(device_allocation_segment<I, N, BufferConfigs, RenderProcessT> const& buff, pipeline_layout<shader_stage::compute, T, BufferConfigs, AssetHeapConfigs> const& layout) const noexcept;

		template<sl::index_t I, asset_heap_config Config, typename RenderProcessT, shader_stage_flags_t ShaderStages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
		void bind_asset_heap(asset_heap_allocation<I, Config, RenderProcessT> const& heap, pipeline_layout<ShaderStages, T, BufferConfigs, AssetHeapConfigs> const& layout) const noexcept;
    	
		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
		void bind_pipeline(pipeline<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& p) const noexcept;

	public:
        inline void begin_draw(std::span<const VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo const& depth_attachment, rect<std::uint32_t> render_area_bounds, rect<float> viewport_bounds, rect<std::uint32_t> scissor_bounds) const noexcept;
        inline void end_draw() const noexcept;

    public:
		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void draw(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_command_buffer_offsets = {}, 
			sl::array<decltype(T::draw_buffers)::size(), sl::uoffset_t> draw_count_buffer_offsets = {} 
		) const noexcept;

		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void dispatch(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			sl::array<decltype(T::dispatch_buffers)::size(), sl::uoffset_t> buffer_offsets = {}
		) const noexcept;
        
    public:
		template<sl::index_t I, sl::index_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
        void copy(device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst, device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src, std::span<const VkBufferCopy> copy_regions) const noexcept;
		template<sl::index_t I, sl::index_t J, sl::size_t N, buffer_config_table<N> BufferConfigs, typename RenderProcessT>
        void copy(device_allocation_segment<I, N, BufferConfigs, RenderProcessT>& dst, device_allocation_segment<J, N, BufferConfigs, RenderProcessT> const& src, std::size_t size, sl::uoffset_t dst_offset = 0, sl::uoffset_t src_offset = 0) const noexcept;

    public:
        inline void pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept;
        inline void image_transition(image& img, VkImageLayout new_layout, VkImageLayout old_layout, std::uint32_t image_count = 1) const noexcept;
    
	private:
        constexpr static frozen::unordered_map<VkImageLayout, std::pair<VkPipelineStageFlagBits2, VkAccessFlagBits2>, 4> image_barrier_map {
            {VK_IMAGE_LAYOUT_UNDEFINED,                {VK_PIPELINE_STAGE_2_NONE,                VK_ACCESS_2_NONE}},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_WRITE_BIT}},
        };

    private:
        std::shared_ptr<logical_device> logi_device_ptr;
        physical_device* phys_device_ptr;
        std::shared_ptr<command_pool> cmd_pool_ptr;
    };
}

#include "sirius/vulkan/core/command_buffer.inl"