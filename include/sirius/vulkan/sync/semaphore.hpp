#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "sirius/core/render_stage.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkSemaphore);

namespace acma::vk {
    struct semaphore : vulkan_ptr<VkSemaphore, vkDestroySemaphore> {
        static result<semaphore> create(std::shared_ptr<logical_device> device, VkSemaphoreType semaphore_type = VK_SEMAPHORE_TYPE_BINARY) noexcept;

	public:
		result<void> wait(sl::uint64_t value, sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()) const noexcept;
    };
}

namespace acma::vk {
    struct semaphore_submit_info : public VkSemaphoreSubmitInfo {
        constexpr semaphore_submit_info() noexcept = default;
        constexpr semaphore_submit_info(VkSemaphore vk_semaphore, render_stage_flags_t stage_flags, std::uint64_t value = 0) noexcept :
            VkSemaphoreSubmitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, vk_semaphore, value, stage_flags, 0} {}
    };
}