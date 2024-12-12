#pragma once
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/traits/shader_traits.hpp"
#include "Duo2D/shaders/rect.hpp"
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct styled_rect;
    template<> struct shader_traits<styled_rect> {
        using shader_data_type = shaders::rect;
        using uniform_type = extent2;
        constexpr static bool instanced = true;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
    template<> struct renderable_traits<styled_rect> {
        using shader_type = styled_rect;
        using index_type = std::uint16_t;
        using vertex_type = void;
        using instance_type = styled_rect;
        constexpr static std::size_t vertex_count = 0;
        constexpr static std::size_t index_count = 6;
        __D2D_DECLARE_NESTED_TRAITS(shader_traits<shader_type>);
    };
}


namespace d2d {
    struct styled_rect {
        rect<float> bounds;
        true_color color; //TEMP: only color, replace with style


    public:
        __D2D_DECLARE_RENDERABLE_TRAITS(renderable_traits<styled_rect>);

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr instance_type instance() const noexcept { return *this; }

    public:
        consteval static std::array<VkVertexInputBindingDescription, 1> binding_descs() noexcept {
            return {{{0, sizeof(styled_rect), VK_VERTEX_INPUT_RATE_INSTANCE}}};
        };

        consteval static std::array<VkVertexInputAttributeDescription, 3> attribute_descs() noexcept {
            return {{
                {0, 0, decltype(decltype(bounds)::pos)::format, offsetof(styled_rect, bounds.pos)},
                {1, 0, decltype(decltype(bounds)::size)::format, offsetof(styled_rect, bounds.size)},
                {2, 0, decltype(color)::format, offsetof(styled_rect, color)},
            }};
        }
    };
}
