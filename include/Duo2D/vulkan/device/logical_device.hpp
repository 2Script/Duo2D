#pragma once
#include "Duo2D/vulkan/device/physical_device.hpp"
#include "Duo2D/vulkan/core/vulkan_ptr.hpp"
#include "Duo2D/traits/vk_traits.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS(VkDevice);

namespace d2d::vk {
    struct logical_device : vulkan_ptr<VkDevice, vkDestroyDevice> {
        static result<logical_device> create(std::weak_ptr<physical_device> associated_phys_device) noexcept;

    public:
        //May need to be per-window instead of per-device?
        std::array<VkQueue, queue_family::num_families> queues;
    };
}
