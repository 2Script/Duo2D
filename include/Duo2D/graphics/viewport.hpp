#pragma once
#include "Duo2D/prim/rect.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d {
    struct viewport : public VkViewport {
        using value_type = decltype(std::declval<VkViewport>().x);

        constexpr viewport() noexcept = default;
        constexpr viewport(value_type x, value_type y, value_type width, value_type height, value_type minDepth = 0.f, value_type maxDepth = 1.f) noexcept :
            VkViewport{x, y, width, height, minDepth, maxDepth} {}
        constexpr viewport(point2<value_type> pos, size2<value_type> size, value_type minDepth = 0.f, value_type maxDepth = 1.f) noexcept :
            VkViewport{pos.x(), pos.y(), size.width(), size.height(), minDepth, maxDepth} {}
        constexpr viewport(rect<value_type> bounds, value_type minDepth = 0.f, value_type maxDepth = 1.f) noexcept :
            VkViewport{bounds.x(), bounds.y(), bounds.width(), bounds.height(), minDepth, maxDepth} {}

        constexpr VkViewport* operator&() noexcept { return this; }
        constexpr VkViewport const* operator&() const noexcept { return this; }
    };
}