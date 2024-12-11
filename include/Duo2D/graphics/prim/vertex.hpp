#pragma once
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/shaders/vertex2.hpp"
#include "Duo2D/shaders/shader_traits.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct vertex2;
    template<> struct shader_traits<vertex2> {
        using shader_data_type = shaders::vertex2;
        //TEMP
        struct uniform_type {
            vk_mat4 model;
            vk_mat4 view;
            vk_mat4 proj;
        };
        constexpr static bool instanced = false;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    };
}


namespace d2d {
    struct vertex2 {
        point2f pos;
        vec4<float> color;

    public:
        __D2D_DECLARE_SHADER_TRAITS(shader_traits<vertex2>);

        consteval static std::array<VkVertexInputBindingDescription, 1> binding_descs() noexcept {
            return {{{0, sizeof(vertex2), VK_VERTEX_INPUT_RATE_VERTEX}}};
        };

        consteval static std::array<VkVertexInputAttributeDescription, 2> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(pos)::format, offsetof(vertex2, pos)},
                {1, 0, decltype(color)::format, offsetof(vertex2, color)}
            }};
        }


    };
}
