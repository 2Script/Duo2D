#pragma once
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/graphics/core/renderable.hpp"
#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/traits/attribute_traits.hpp"
#include "Duo2D/arith/size.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/shaders/rect.hpp"
#include "Duo2D/vulkan/memory/attribute.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct styled_rect;
    struct transform2 { //TEMP
        vec2<float> scale;
        vk_mat2 rotation;
        vec2<float> translation;
    };
    template<> struct renderable_traits<styled_rect> {
        using shader_type = styled_rect;
        constexpr static auto vert_shader_data = std::to_array(shaders::rect::vert);
        constexpr static auto frag_shader_data = std::to_array(shaders::rect::frag);
        using uniform_type = extent2;
        using push_constant_types = std::tuple<extent2&>;
        using attribute_types = vk::make_attribute_types_t<true_color, transform2, std::uint32_t, rect<std::uint32_t>>;
        using asset_type = texture;
        using index_type = std::uint16_t;
        struct instance_type {
            pt2<float> pos;
            size2<float> size;
        }; 

        constexpr static std::size_t max_texture_count = 2;
        constexpr static std::size_t index_count = 6;

        constexpr static VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        constexpr static VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    };
}


namespace d2d {
    struct styled_rect : renderable<styled_rect> {
        constexpr styled_rect() noexcept :
            bounds(rect<float>{}), color(true_color{}), transform(transform2{}), border_width(0), texture_bounds(rect<std::uint32_t>{}) {}
        constexpr styled_rect(rect<float> _bounds, true_color _color, transform2 _xfrm = {}, std::uint32_t _border_width = 0, rect<std::uint32_t> _tex_bounds = {}) noexcept :
            bounds(_bounds), color(_color), transform(_xfrm), border_width(_border_width), texture_bounds(_tex_bounds) {};

    public:
        rect<float> bounds;

        //TODO (TEMP): replace with style
        vk::attribute<true_color> color;
        vk::attribute<transform2> transform;
        vk::attribute<std::uint32_t> border_width;
        vk::attribute<rect<std::uint32_t>> texture_bounds;

    public:
        consteval static std::array<index_type, index_count> indices() noexcept { return {0, 1, 2, 2, 1, 3}; }
        constexpr instance_type instance() const noexcept { return std::bit_cast<instance_type>(bounds); }
        constexpr attribute_types attributes() noexcept { return std::tie(color, transform, border_width, texture_bounds); }
    
    public:
        template<typename... Ts, typename UniformT, std::size_t N>
        constexpr static void on_swap_chain_update(basic_window<Ts...> const& win, std::span<UniformT, N> uniform_map) noexcept {
            for(std::size_t i = 0; i < N; ++i) std::memcpy(&uniform_map[i], &win.swap_chain().extent(), sizeof(UniformT));
        }
    };
}

namespace d2d {
    struct clone_rect : styled_rect {};
    template<> struct renderable_traits<clone_rect> : public renderable_traits<styled_rect> {};
}