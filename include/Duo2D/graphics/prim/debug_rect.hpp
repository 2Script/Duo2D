#pragma once
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/shaders/debug_rect.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <cstddef>
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct debug_rect;
    template<> struct shader_traits<debug_rect> {
        using shader_type = debug_rect;
        using shader_data_type = shaders::debug_rect;
        using uniform_type = extent2;
        using attribute_types = make_attribute_types_t<transform2, std::uint32_t>;
        constexpr static bool instanced = false;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
    template<> struct renderable_traits<debug_rect> : shader_traits<debug_rect> {
        using index_type = std::uint16_t;
        struct vertex_type {
            pt2<float> pos;
            true_color color;
        };
        constexpr static std::size_t index_count = 6;
        constexpr static std::size_t vertex_count = 4;
    };
}


namespace d2d {
    struct debug_rect : renderable<debug_rect> {
        rect<float> bounds;
        true_color color; //TEMP: only color, replace with style
        attribute<transform2> transform = {};
        attribute<std::uint32_t> border_width = {};

    public:
        constexpr std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr std::array<vertex_type, vertex_count> vertices() const noexcept {
            std::array<vertex_type, vertex_count> ret = {};
            for(std::size_t i = 0; i < vertex_count; ++i)
                ret[i] = {bounds.points()[i], color};
            return ret; 
        }
        constexpr attribute_types attributes() noexcept { return std::tie(transform, border_width); }

    public:
        consteval static std::array<VkVertexInputBindingDescription, 2> binding_descs() noexcept {
            return {{
                {0, sizeof(vertex_type), VK_VERTEX_INPUT_RATE_VERTEX},
                {1, sizeof(transform2) + sizeof(std::uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE}
            }};
        };

        consteval static std::array<VkVertexInputAttributeDescription, 6> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(decltype(bounds)::pos)::format, offsetof(vertex_type, pos)},
                {1, 0, decltype(color)::format, offsetof(vertex_type, color)},

                //TEMP harcoded attributes
                {2, 1, decltype(transform2::scale)::format,       0},
                {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT,             sizeof(transform2::scale)},
                {5, 1, decltype(transform2::translation)::format, sizeof(transform2::scale) + 2 * sizeof(vec2<float>)},
                {6, 1, VK_FORMAT_R32_UINT,                        sizeof(transform2::scale) + 2 * sizeof(vec2<float>) + sizeof(transform2::translation)},
            }};
        }
    };
}