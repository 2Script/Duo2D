#pragma once
#include "Duo2D/prim/point.hpp"
#include "Duo2D/prim/vector.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct vertex2 {
        point2f pos;
        vec4<float> color;

    public:
        constexpr static VkVertexInputBindingDescription binding_desc() noexcept {
            return {0, sizeof(vertex2), VK_VERTEX_INPUT_RATE_VERTEX};
        }

        constexpr static std::array<VkVertexInputAttributeDescription, 2> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(pos)::format, offsetof(vertex2, pos)},
                {1, 0, decltype(color)::format, offsetof(vertex2, color)}
            }};
        }
    };
}