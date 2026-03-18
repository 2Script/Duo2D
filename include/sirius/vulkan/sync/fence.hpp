#pragma once
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS_DEVICE(VkFence);

namespace acma::vk {
    struct fence : vulkan_ptr<VkFence, vkDestroyFence> {
        static result<fence> create(std::shared_ptr<logical_device> device) noexcept;

    public:
        result<void> wait(std::uint64_t timeout = UINT64_MAX) const noexcept;
        result<void> reset() const noexcept;
    };
}