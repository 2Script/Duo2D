#pragma once

#include <vulkan/vulkan.h>


namespace acma::vk::impl {
	constexpr VkInstance& vulkan_instance() noexcept {
		static VkInstance i{};
		return i;
	}
}
