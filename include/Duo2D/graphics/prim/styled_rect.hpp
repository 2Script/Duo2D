#pragma once
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/traits/renderable_constraints.hpp"
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
        using attribute_types = make_attribute_types_t<transform2, std::uint32_t, rect<std::uint32_t>>;
        constexpr static std::size_t max_texture_count = 2;
        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
    template<> struct renderable_traits<styled_rect> : shader_traits<styled_rect> {
        using index_type = std::uint16_t;
        struct instance_type {
            pt2<float> pos;
            size2<float> size;
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
        attribute<rect<std::uint32_t>> texture_bounds = {};

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr instance_type instance() const noexcept { return {bounds.pos, bounds.size, color}; }
        constexpr attribute_types attributes() noexcept { return std::tie(transform, border_width, texture_bounds); }
    };
    
    struct clone_rect : styled_rect {};
}
