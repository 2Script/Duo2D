#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d::impl {
    template<std::size_t Dims>
    concept Cartesian = Dims == 2 || Dims == 3;

    template<std::size_t Dims, typename T, bool HoldsSize>
    concept VkCompatibleType = Cartesian<Dims> && (
        (std::is_convertible_v<T, decltype(std::declval<VkOffset2D>().x)> && !HoldsSize) ||
        (std::is_convertible_v<T, decltype(std::declval<VkExtent2D>().height)> && HoldsSize)
    ); 


    template<std::size_t Dims, bool HoldsSize>
    using vk_vector_type = std::conditional_t<!HoldsSize,
        std::conditional_t<Dims == 2, VkOffset2D, VkOffset3D>, 
        std::conditional_t<Dims == 2, VkExtent2D, VkExtent3D>
    >;

    template<std::size_t Dims, bool HoldsSize>
    using vk_component_type = std::conditional_t<!HoldsSize,
        decltype(std::declval<VkOffset2D>().x), 
        decltype(std::declval<VkExtent2D>().height)
    >;
}


namespace d2d {
    template<std::size_t Dims, typename UnitTy, bool HoldsSize = false>
    struct vector {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
        //aggregate
        std::array<UnitTy, Dims> _elems;

        constexpr explicit operator std::array<UnitTy, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<decltype(_elems)>()))() const noexcept { return std::tuple_cat(_elems); }

        constexpr       UnitTy& operator[](std::size_t pos)       noexcept { return _elems[pos]; }
        constexpr const UnitTy& operator[](std::size_t pos) const noexcept { return _elems[pos]; }


    public:
        constexpr       UnitTy& x()       noexcept requires (impl::Cartesian<Dims> && !HoldsSize) { return _elems[0]; }
        constexpr const UnitTy& x() const noexcept requires (impl::Cartesian<Dims> && !HoldsSize) { return _elems[0]; }
        constexpr       UnitTy& y()       noexcept requires (impl::Cartesian<Dims> && !HoldsSize) { return _elems[1]; }
        constexpr const UnitTy& y() const noexcept requires (impl::Cartesian<Dims> && !HoldsSize) { return _elems[1]; }
        constexpr       UnitTy& z()       noexcept requires (Dims == 3 && !HoldsSize) { return _elems[2]; }
        constexpr const UnitTy& z() const noexcept requires (Dims == 3 && !HoldsSize) { return _elems[2]; }

    public:
        constexpr       UnitTy& width()        noexcept requires (impl::Cartesian<Dims> && HoldsSize) { return _elems[0]; }
        constexpr const UnitTy& width()  const noexcept requires (impl::Cartesian<Dims> && HoldsSize) { return _elems[0]; }
        constexpr       UnitTy& height()       noexcept requires (impl::Cartesian<Dims> && HoldsSize) { return _elems[1]; }
        constexpr const UnitTy& height() const noexcept requires (impl::Cartesian<Dims> && HoldsSize) { return _elems[1]; }
        constexpr       UnitTy& depth()        noexcept requires (Dims == 3 && HoldsSize) { return _elems[2]; }
        constexpr const UnitTy& depth()  const noexcept requires (Dims == 3 && HoldsSize) { return _elems[2]; }


    public:
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept { return {_elems[0], _elems[1]}; }

        constexpr explicit operator impl::vk_vector_type<Dims, HoldsSize>() const noexcept requires (impl::VkCompatibleType<Dims, UnitTy, HoldsSize>) {
            using component_type = impl::vk_component_type<Dims, HoldsSize>;
            if constexpr (Dims == 2)
                return {static_cast<component_type>(_elems[0]), static_cast<component_type>(_elems[1])};
            else 
                return {static_cast<component_type>(_elems[0]), static_cast<component_type>(_elems[1]), static_cast<component_type>(_elems[2])};
        }

    public:
        template<std::size_t OtherDims, typename OtherUnitTy, bool OtherHoldsSize> requires (std::is_convertible_v<UnitTy, OtherUnitTy>)
        constexpr explicit operator vector<OtherDims, OtherUnitTy, OtherHoldsSize>() const noexcept {
            constexpr std::size_t new_dims = OtherDims > Dims ? Dims : OtherDims;
            return [this]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                return vector<OtherDims, OtherUnitTy, OtherHoldsSize>{static_cast<OtherUnitTy>(_elems[I])...};
            }(std::make_index_sequence<new_dims>{});
        }

    };
}


namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using vec = vector<Dims, UnitTy>;

    template<typename UnitTy> using vector2 = vector<2, UnitTy>;
    template<typename UnitTy> using vector3 = vector<3, UnitTy>;
    template<typename UnitTy> using vec2 = vec<2, UnitTy>;
    template<typename UnitTy> using vec3 = vec<3, UnitTy>;
}