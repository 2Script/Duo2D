#pragma once
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS_DEVICE(VkFence);

namespace d2d {
    struct fence : pipeline_obj<VkFence, vkDestroyFence> {
        static result<fence> create(logical_device& device) noexcept;

    public:
        result<void> wait(std::uint64_t timeout = UINT64_MAX);
        result<void> reset();
    };
}
