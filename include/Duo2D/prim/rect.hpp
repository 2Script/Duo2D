#pragma once
#include "Duo2D/prim/point.hpp"
#include "Duo2D/prim/size.hpp"
#include "Duo2D/prim/vector.hpp"
#include <type_traits>
#include <vulkan/vulkan_core.h>


namespace d2d::impl {
    template<typename PosT, typename SizeT>
    concept VkCompatibleRectType = 
        impl::VkCompatibleType<2, PosT, false> && impl::VkCompatibleType<2, SizeT, true>;
}


namespace d2d {
    template<typename T, typename SizeT = T>
    struct rect {
        constexpr rect() noexcept = default;
        constexpr rect(T x, T y, SizeT width, SizeT height) noexcept : pos{x, y}, size{width, height} {}
        constexpr rect(point2<T> p, size2<SizeT> s) noexcept : pos(p), size(s) {}

        constexpr explicit rect(VkOffset2D offset, VkExtent2D extent) noexcept requires impl::VkCompatibleRectType<T, SizeT> : 
            pos{static_cast<T>(offset.x), static_cast<T>(offset.y)}, 
            size{static_cast<SizeT>(extent.width), static_cast<SizeT>(extent.height)} {}
        constexpr explicit rect(VkRect2D r) noexcept requires impl::VkCompatibleRectType<T, SizeT> : rect(r.offset, r.extent) {}

    public:
        constexpr explicit operator VkRect2D() const noexcept requires impl::VkCompatibleRectType<T, SizeT> { 
            return {static_cast<VkOffset2D>(pos), static_cast<VkExtent2D>(size)};
        }

    public:
        constexpr T x() const noexcept { return pos.x(); }
        constexpr T y() const noexcept { return pos.y(); }
        constexpr SizeT width() const noexcept { return size.width(); }
        constexpr SizeT height() const noexcept { return size.height(); }

    public:
        point2<T> pos;
        size2<SizeT> size;
    };
}

namespace d2d {
    using vk_rect = rect<decltype(std::declval<VkOffset2D>().x), decltype(std::declval<VkExtent2D>().height)>;
}