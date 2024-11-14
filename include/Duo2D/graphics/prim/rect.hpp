#pragma once
#include "Duo2D/prim/bounds_rect.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"
#include "Duo2D/prim/point.hpp"
#include "Duo2D/prim/size.hpp"

namespace d2d {
    struct rect {
        bounds_rect<float> bounds;
        true_color color; //TEMP: only color, replace with style

        constexpr std::array<vertex2, 4> verticies(size2<float> screen_size) const noexcept {
            return [this, screen_size]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                vec4<float> normalized_color = color.normalize();
                std::array<point2f, 4> points = bounds.points(screen_size);
                return std::array<vertex2, 4>{{vertex2{points[I], normalized_color}...}};
            }(std::make_index_sequence<4>{});
        }

        constexpr static std::array<std::uint32_t, 6> indicies() noexcept {
            return {0, 1, 2, 2, 3, 0};
        }
    };
}