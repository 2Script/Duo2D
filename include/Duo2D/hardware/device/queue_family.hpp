#pragma once
#include <cstdint>
#include <optional>
#include <array>
#include <vulkan/vulkan.h>

namespace d2d {
    namespace queue_family {
    enum { 
        graphics,
        
        
        present, //requires special construction

        num_families
    };
    }
}

namespace d2d {
    namespace queue_family {
        constexpr std::array<VkQueueFlagBits, queue_family::num_families> vulkan_bit = {
            VK_QUEUE_GRAPHICS_BIT,

            VK_QUEUE_FLAG_BITS_MAX_ENUM
        }; 
    }
}


namespace d2d {
    using queue_family_idxs_t = std::array<std::optional<std::uint32_t>, queue_family::num_families>;
}
