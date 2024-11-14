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

    template<typename PosT, typename SizeT>
    constexpr bool identity_rect = std::is_same_v<PosT, SizeT>;
}


namespace d2d {
    template<typename T, typename SizeT = T>
    struct bounds_rect {
        constexpr bounds_rect() noexcept = default;
        constexpr bounds_rect(T x, T y, SizeT width, SizeT height) noexcept : pos{x, y}, size{width, height} {}
        constexpr bounds_rect(point2<T> p, size2<SizeT> s) noexcept : pos(p), size(s) {}

        constexpr explicit bounds_rect(VkOffset2D offset, VkExtent2D extent) noexcept requires impl::VkCompatibleRectType<T, SizeT> : 
            pos{static_cast<T>(offset.x), static_cast<T>(offset.y)}, 
            size{static_cast<SizeT>(extent.width), static_cast<SizeT>(extent.height)} {}
        constexpr explicit bounds_rect(VkRect2D r) noexcept requires impl::VkCompatibleRectType<T, SizeT> : bounds_rect(r.offset, r.extent) {}

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
        constexpr std::array<point2<T>, 4> points() const noexcept requires (impl::identity_rect<T, SizeT>) { return {top_left(), top_right(), bottom_right(), bottom_left()}; }
        constexpr std::array<point2<T>, 4> points(size2<T> normalize_to) const noexcept requires (impl::identity_rect<T, SizeT>) { 
            constexpr point2<T> twos = {2,2}, ones = {1,1};
            return {
                (twos * (top_left()/normalize_to)) - ones, (twos * (top_right()/normalize_to)) - ones, 
                (twos * (bottom_right()/normalize_to)) - ones, (twos * (bottom_left()/normalize_to)) - ones, 
            }; 
        }
        constexpr point2<T> operator[](std::size_t pos) const noexcept requires (impl::identity_rect<T, SizeT>) { return points()[pos]; }
        
        constexpr point2<T> top_left()     const noexcept requires (impl::identity_rect<T, SizeT>) { return pos; }
        constexpr point2<T> top_right()    const noexcept requires (impl::identity_rect<T, SizeT>) { return {pos.x() + size.width(), pos.y()}; }
        constexpr point2<T> bottom_right() const noexcept requires (impl::identity_rect<T, SizeT>) { return (pos + size); }
        constexpr point2<T> bottom_left()  const noexcept requires (impl::identity_rect<T, SizeT>) { return {pos.x(), pos.y() + size.height()}; }

    public:
        point2<T> pos;
        size2<SizeT> size;
    };
}

namespace d2d {
    using vk_rect = bounds_rect<decltype(std::declval<VkOffset2D>().x), decltype(std::declval<VkExtent2D>().height)>;
}