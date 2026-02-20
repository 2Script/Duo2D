#pragma once
#include <span>

#include <vulkan/vulkan.h>

#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/pipeline_layout.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkPipeline);

namespace d2d::vk {
    template<typename T, sl::size_t N, resource_table<N> Resources>
    struct pipeline : vulkan_ptr<VkPipeline, vkDestroyPipeline> {
        static result<pipeline> create(std::shared_ptr<logical_device> device, std::span<const VkFormat> color_attachment_formats, VkFormat depth_attachment_format) noexcept;

	public:
		constexpr auto&& layout(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._layout); }

	private:
		pipeline_layout<T, N, Resources> _layout;
    };
}

#include "Duo2D/vulkan/memory/pipeline.inl"