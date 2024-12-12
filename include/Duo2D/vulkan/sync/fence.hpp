#pragma once
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS_DEVICE(VkFence);

namespace d2d {
    struct fence : vulkan_ptr<VkFence, vkDestroyFence> {
        static result<fence> create(logical_device& device) noexcept;

    public:
        result<void> wait(std::uint64_t timeout = UINT64_MAX) const noexcept;
        result<void> reset() const noexcept;
    };
}
