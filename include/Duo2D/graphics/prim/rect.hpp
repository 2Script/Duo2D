#pragma once
#include "Duo2D/prim/bounds_rect.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/graphics/prim/vertex.hpp"
#include "Duo2D/prim/point.hpp"

namespace d2d {
    struct rect {
        bounds_rect<float> bounds;
        true_color color; //TEMP: only color, replace with style

        constexpr std::array<vertex2, 4> verticies() noexcept {
            return [this]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                vec4<float> normalized_color = color.normalize();
                std::array<point2f, 4> points = bounds.points();
                return std::array<vertex2, 4>{{vertex2{points[I], normalized_color}...}};
            }(std::make_index_sequence<4>{});
        }
    };
}