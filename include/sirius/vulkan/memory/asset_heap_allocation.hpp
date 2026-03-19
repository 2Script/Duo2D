#pragma once

#include <vulkan/vulkan.h>

#include "sirius/core/asset_heap_config.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/display/image_sampler.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/vulkan/memory/device_allocation_segment.hpp"
#include "sirius/vulkan/memory/generic_allocation.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/memory/descriptor_info.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorSetLayout);
__D2D_DECLARE_VK_TRAITS_DEVICE(VkDescriptorPool);

namespace acma::vk {
	using descriptor_set_layout_type = vulkan_ptr<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout>;
}

namespace acma::vk {
	template<sl::index_t I, asset_heap_config Config, typename RenderProcessT>
	class asset_heap_allocation : public generic_allocation<Config.coupling, RenderProcessT> {
	public:
		static result<asset_heap_allocation> create(
			std::shared_ptr<logical_device> logi_device,
			std::shared_ptr<physical_device> phys_device//,
			//std::shared_ptr<command_pool> transfer_command_pool
		) noexcept;
	public:
		using generic_allocation<Config.coupling, RenderProcessT>::allocation_count;

		using typename generic_allocation<Config.coupling, RenderProcessT>::memory_ptr_type;
		using image_ptr_type = vulkan_ptr<VkImage, vkDestroyImage>;

	private:
	 	template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		using buffer_segment = device_allocation_segment<J, N, BufferConfigs, RenderProcessT>;

	private:
		struct image_allocation {
			sl::uoffset_t offset;
			image img;
		};	
	private:
		using descriptor_pool_type = vulkan_ptr<VkDescriptorPool, vkDestroyDescriptorPool>;
		using descriptor_set_type = vulkan_ptr_base<VkDescriptorSet>;


	public:
		constexpr sl::size_t total_size() const noexcept { return _images.size() + _sampler_infos.size(); }
		//constexpr sl::size_t capacity() const noexcept { return allocated_bytes; }

	public:
		constexpr sl::array<asset_usage_policy::num_usage_policies, descriptor_set_type> const& descirptor_sets() const& noexcept { return _descriptor_sets[this->allocation_index()]; }

	public:
		template<typename T>
		constexpr result<void> push_back(T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, T&&>)
		requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, T&&>);

		template<typename... Args>
		constexpr result<void> emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, Args&&...>)
		requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, Args&&...>);
		
	public:
	 	// template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		// constexpr result<void> emplace_back(buffer_segment<J, N, BufferConfigs> const& uniform_buffer) noexcept
		// requires((buffer_segment<J, N, BufferConfigs>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform);

	 	// template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		// constexpr result<void> try_emplace_back(buffer_segment<J, N, BufferConfigs> const& uniform_buffer) noexcept
		// requires((buffer_segment<J, N, BufferConfigs>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform);

	public:
	 	template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		constexpr result<void> emplace_back(buffer_segment<J, N, BufferConfigs> const& texture_data_buffer) noexcept
		requires((buffer_segment<J, N, BufferConfigs>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data);

	 	template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		constexpr result<void> try_emplace_back(buffer_segment<J, N, BufferConfigs> const& texture_data_buffer) noexcept
		requires((buffer_segment<J, N, BufferConfigs>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data);

	public:
		constexpr result<void> reserve(sl::size_t image_capacity_bytes) noexcept;
		constexpr result<void> reserve(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept;
	public:
		constexpr void clear() noexcept;
		
		constexpr result<void> resize(sl::size_t image_size_bytes) noexcept;
		constexpr result<void> try_resize(sl::size_t image_size_bytes) noexcept;

		constexpr result<void> resize(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept;
		constexpr result<void> try_resize(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept;


	private:
		constexpr result<std::unique_ptr<image_allocation[]>> make_images(
			std::vector<texture_data_info> const& texture_data_infos,
			sl::size_t& total_size_out
		) noexcept;
		constexpr result<void> initialize_image_views(
			sl::index_t alloc_idx
		) noexcept;

		constexpr result<void> bind(image_allocation&& img_alloc, sl::index_t alloc_idx) noexcept;
		constexpr void update_descriptors(sl::index_t alloc_idx) noexcept;

	private:
		result<void> make_pools(
			sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts,
			sl::index_t alloc_idx
		) noexcept;

	 	template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		result<void> upload_image_data(
			buffer_segment<J, N, BufferConfigs> const& texture_data_buffer,
			sl::index_t alloc_idx,
			sl::index_t image_start_idx,
			sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
		) noexcept;

	protected:
		template<sl::index_t J>
		result<void> realloc(sl::index_constant_type<J>, sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires(J == I);


	private:
		constexpr sl::index_t allocation_index() const& noexcept {
			return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
		}

	private:
		template<sl::index_t, sl::size_t N, buffer_config_table<N>, typename>
		friend class device_allocation_segment;

    	template<shader_stage_flags_t, typename, auto, auto>
		friend struct pipeline_layout;

		friend struct command_buffer;
	private:
		sl::size_t allocated_bytes, data_bytes, desired_bytes;
		sl::array<allocation_count, std::vector<image>> _images;
		sl::array<allocation_count, std::unique_ptr<image_view[]>> _image_views;
		sl::array<allocation_count, std::vector<asset_usage_policy_t>> _image_usages;
		sl::array<allocation_count, std::vector<image_sampler>> _samplers;
		sl::array<allocation_count, std::vector<VkSamplerCreateInfo>> _sampler_infos;
		
		sl::array<allocation_count, descriptor_pool_type> _descriptor_pools;
		sl::array<allocation_count, sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t>> _descriptor_counts;
		sl::array<allocation_count, sl::array<asset_usage_policy::num_usage_policies, descriptor_set_type>> _descriptor_sets;

		sl::array<asset_usage_policy::num_usage_policies, descriptor_set_layout_type> _descriptor_set_layouts;
		//sl::array<asset_usage_policy::num_usage_policies, VkWriteDescriptorSet> _descriptor_writes;
	};
}

#include "sirius/vulkan/memory/asset_heap_allocation.inl"