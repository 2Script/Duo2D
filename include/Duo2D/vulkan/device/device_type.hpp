#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>


namespace d2d::vk {
    enum class device_type : std::uint32_t {
        other,
        integrated_gpu,
        discrete_gpu,
        virtual_gpu,
        cpu,

        unknown = VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM,
    };
}
