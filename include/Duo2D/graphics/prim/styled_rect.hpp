#pragma once
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/graphics/prim/renderable_traits.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/shaders/shader_traits.hpp"
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
    struct old_rect;
    template<> struct renderable_traits<old_rect> {
        using shader_type = vertex2;
        using index_type = std::uint16_t;
        using vertex_type = vertex2;
        using instance_type = void;
        constexpr static std::size_t vertex_count = 4;
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

    struct old_rect {
        rect<float> bounds;
        true_color color; //TEMP: only color, replace with style
        std::uint16_t idx;

        __D2D_DECLARE_RENDERABLE_TRAITS(renderable_traits<old_rect>);

        constexpr std::array<vertex2, vertex_count> vertices(size2<float> screen_size = {1600,900}) const noexcept {
            return [this, screen_size]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                vec4<float> normalized_color = color.normalize();
                vec4<vec4<float>> colors = {normalized_color, {1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}};
                std::array<point2f, 4> points = bounds.points(screen_size);
                return std::array<vertex2, 4>{{vertex2{points[I], colors[I]}...}};
            }(std::make_index_sequence<4>{});
        }

        constexpr std::array<index_type, index_count> indices() const noexcept {
            constexpr std::array<index_type, index_count> idx_base = {0, 1, 2, 2, 3, 0};
            std::array<index_type, index_count> ret;
            for(std::size_t i = 0; i < index_count; ++i)
                ret[i] = idx_base[i] + (idx * vertex_count);
            return ret;
        }
    };
}
