#pragma once
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/shaders/rect.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <cstddef>
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct styled_rect;
    struct transform2 { //TEMP
        vec2<float> scale;
        vk_mat2 rotation;
        vec2<float> translation;
    };
    template<> struct shader_traits<styled_rect> {
        using shader_type = styled_rect;
        using shader_data_type = shaders::rect;
        using uniform_type = extent2;
        using push_constant_types = std::tuple<extent2&>;
        using attribute_types = make_attribute_types_t<transform2, std::uint32_t>;
        constexpr static bool instanced = true;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
    template<> struct renderable_traits<styled_rect> : shader_traits<styled_rect> {
        using index_type = std::uint16_t;
        struct instance_type { //TEMP
            rect<float> bounds;
            true_color color;
        }; 
        constexpr static std::size_t index_count = 6;
    };
}


namespace d2d {
    struct styled_rect : renderable<styled_rect> {
        rect<float> bounds;
        true_color color; //TEMP: only color, replace with style
        attribute<transform2> transform = {};
        attribute<std::uint32_t> border_width = {};

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr instance_type instance() const noexcept { return {bounds, color}; }
        constexpr attribute_types attributes() noexcept { return std::tie(transform, border_width); }

    public:
        consteval static std::array<VkVertexInputBindingDescription, 2> binding_descs() noexcept {
            return {{
                {0, sizeof(instance_type), VK_VERTEX_INPUT_RATE_INSTANCE},
                {1, sizeof(transform2) + sizeof(std::uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE}
            }};
        };

        consteval static std::array<VkVertexInputAttributeDescription, 7> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(decltype(bounds)::pos)::format, offsetof(styled_rect, bounds.pos)},
                {1, 0, decltype(decltype(bounds)::size)::format, offsetof(styled_rect, bounds.size)},
                {2, 0, decltype(color)::format, offsetof(styled_rect, color)},

                //TEMP harcoded attributes
                {3, 1, decltype(transform2::scale)::format,       0},
                {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT,             sizeof(transform2::scale)},
                {6, 1, decltype(transform2::translation)::format, sizeof(transform2::scale) + 2 * sizeof(vec2<float>)},
                {7, 1, VK_FORMAT_R32_UINT,                        sizeof(transform2::scale) + 2 * sizeof(vec2<float>) + sizeof(transform2::translation)},
            }};
        }
    };


    constexpr bool x = impl::has_push_constants_v<styled_rect>;
}