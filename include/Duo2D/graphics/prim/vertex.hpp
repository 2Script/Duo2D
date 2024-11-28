#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/logical_device.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include "Duo2D/graphics/pipeline/shader_module.hpp"
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/shaders/vertex2.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct vertex2 {
        point2f pos;
        vec4<float> color;

    public:
        consteval static VkVertexInputBindingDescription binding_desc() noexcept {
            return {0, sizeof(vertex2), VK_VERTEX_INPUT_RATE_VERTEX};
        }

        consteval static std::array<VkVertexInputAttributeDescription, 2> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(pos)::format, offsetof(vertex2, pos)},
                {1, 0, decltype(color)::format, offsetof(vertex2, color)}
            }};
        }

        using shader_type = shaders::vertex2;

        //TEMP
        struct uniform_type {
            vk_mat4 model;
            vk_mat4 view;
            vk_mat4 proj;
        };
    };
}


namespace d2d::impl {
    template<typename T>
    concept RenderableType = requires (T t) { 
        {t.verticies(std::declval<d2d::size2f>())} noexcept; 
        {T::indicies()} noexcept;
    };

    template<typename T>
    concept ShaderType = requires {
        {T::binding_desc()} noexcept;
        {T::attribute_descs()} noexcept;
        typename T::shader_type;
    };
}