#pragma once
#include "Duo2D/graphics/pipeline/physical_device.hpp"
#include "Duo2D/graphics/pipeline/pipeline_obj.hpp"
#include "Duo2D/traits/vk_traits.hpp"
#include <vulkan/vulkan_core.h>


__D2D_DECLARE_VK_TRAITS(VkDevice);

namespace d2d {
    struct logical_device : pipeline_obj<VkDevice, vkDestroyDevice> {
        static result<logical_device> create(physical_device& associated_phys_device) noexcept;

    public:
        present_mode mode;
        display_format format;
        //May need to be per-window instead of per-device?
        std::array<VkQueue, queue_family::num_families> queues;
    };
}
