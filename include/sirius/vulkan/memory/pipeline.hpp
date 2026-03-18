#pragma once
#include <span>

#include <vulkan/vulkan.h>

#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"
#include "sirius/vulkan/memory/pipeline_layout.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipeline);

namespace acma::vk {
	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline;
}


namespace acma::vk {
    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline<bind_point::graphics, T, BufferConfigs, AssetHeapConfigs> : vulkan_ptr<VkPipeline, vkDestroyPipeline> {
		template<typename RenderProcessT>
        static result<pipeline> create(std::shared_ptr<logical_device> logi_device, RenderProcessT&& proc, std::span<const VkFormat> color_attachment_formats, VkFormat depth_attachment_format) noexcept;

	public:
		constexpr auto&& layout(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._layout); }

	private:
		pipeline_layout<shader_stage::all_graphics, T, BufferConfigs, AssetHeapConfigs> _layout;
    };


    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline<bind_point::compute, T, BufferConfigs, AssetHeapConfigs> : vulkan_ptr<VkPipeline, vkDestroyPipeline> {
		template<typename RenderProcessT>
        static result<pipeline> create(std::shared_ptr<logical_device> logi_device, RenderProcessT&& proc) noexcept;

	public:
		constexpr auto&& layout(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._layout); }

	private:
		pipeline_layout<shader_stage::compute, T, BufferConfigs, AssetHeapConfigs> _layout;
    };
}

#include "sirius/vulkan/memory/pipeline.inl"