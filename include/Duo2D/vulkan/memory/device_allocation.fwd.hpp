#pragma once
#include <vulkan/vulkan.h>

#include "Duo2D/core/resource_table.hpp"
#include "Duo2D/traits/vk_traits.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkDeviceMemory);

namespace d2d::vk {
    template<sl::size_t FramesInFlight, typename IndexSeq, buffering_policy_t BufferPolicy, memory_policy_t MemoryPolicy, typename Derived>
    class device_allocation;
}
