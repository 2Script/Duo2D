#pragma once
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/graphics/prim/styled_rect.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/shaders/debug_rect.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <cstddef>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct debug_rect;
    template<> struct renderable_traits<debug_rect> {
        using shader_type = debug_rect;
        constexpr static auto vert_shader_data = std::to_array(shaders::debug_rect::vert);
        constexpr static auto frag_shader_data = std::to_array(shaders::debug_rect::frag);
        using uniform_type = extent2;
        using attribute_types = vk::make_attribute_types_t<transform2, std::uint32_t>;
        using index_type = std::uint16_t;
        struct vertex_type {
            pt2<float> pos;
            true_color color;
        };

        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
}


namespace d2d {
    struct debug_rect : public renderable<debug_rect> {
        rect<float> bounds;
        true_color color; //TEMP: only color, replace with style
        vk::attribute<transform2> transform = {};
        vk::attribute<std::uint32_t> border_width = {};

    public:
        constexpr std::vector<index_type> indices() const noexcept { return {0, 1, 2, 2, 3, 0}; }
        constexpr std::vector<vertex_type> vertices() const noexcept {
            constexpr std::size_t vertex_count = 4;
            std::vector<vertex_type> ret(vertex_count);
            for(std::size_t i = 0; i < vertex_count; ++i)
                ret[i] = {bounds.points()[i], color};
            return ret; 
        }
        constexpr attribute_types attributes() noexcept { return std::tie(transform, border_width); }
    
    public:
        template<typename... Ts, typename UniformT, std::size_t N>
        constexpr static void on_swap_chain_update(basic_window<Ts...> const& win, std::span<UniformT, N> uniform_map) noexcept {
            for(std::size_t i = 0; i < N; ++i) std::memcpy(&uniform_map[i], &win.swap_chain().extent(), sizeof(UniformT));
        }
    };
}