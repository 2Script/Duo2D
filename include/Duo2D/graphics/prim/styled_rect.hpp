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
        inline static extent2*& swap_extent() { static extent2* e; return e; }
    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr instance_type instance() const noexcept { return {bounds, color}; }
        constexpr attribute_types attributes() noexcept { return std::tie(transform, border_width); }
        inline static push_constant_types push_constants() noexcept { return std::tie(*swap_extent()); }

    public:
        consteval static std::array<VkVertexInputBindingDescription, 2> binding_descs() noexcept {
            return {{
                {0, sizeof(instance_type), VK_VERTEX_INPUT_RATE_INSTANCE},
                {1, sizeof(transform2) + sizeof(std::uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE}
            }};
        };

        consteval static std::array<VkVertexInputAttributeDescription, 8> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(decltype(bounds)::pos)::format, offsetof(styled_rect, bounds.pos)},
                {1, 0, decltype(decltype(bounds)::size)::format, offsetof(styled_rect, bounds.size)},
                {2, 0, decltype(color)::format, offsetof(styled_rect, color)},

                //TEMP harcoded attributes
                {3, 1, decltype(transform2::scale)::format,       0},
                {4, 1, vec2<float>::format,                       sizeof(transform2::scale)},
                {5, 1, vec2<float>::format,                       sizeof(transform2::scale) + sizeof(vec2<float>)},
                {6, 1, decltype(transform2::translation)::format, sizeof(transform2::scale) + 2 * sizeof(vec2<float>)},
                {7, 1, VK_FORMAT_R32_UINT,                        sizeof(transform2::scale) + 2 * sizeof(vec2<float>) + sizeof(transform2::translation)},
            }};
        }

        consteval static std::array<VkPushConstantRange, 1> push_constant_ranges() noexcept {
            return {{{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(extent2)}}};
        }
    };
}

            //constexpr static std::size_t attribute_size = d2d::impl::extract_attribute_size<typename d2d::styled_rect::attribute_types>::value;
            //constexpr static std::size_t a = sizeof(d2d::transform2);
            //constexpr static std::size_t b = sizeof(std::uint32_t);