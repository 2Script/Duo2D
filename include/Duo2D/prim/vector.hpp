#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d {
    template<template<std::size_t, typename> typename DerivedTy, std::size_t Dims, typename UnitTy>
    struct vector_base {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
        //aggregate
        std::array<UnitTy, Dims> _elems;

        constexpr explicit operator std::array<UnitTy, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<decltype(_elems)>()))() const noexcept { return std::tuple_cat(_elems); }

        constexpr       UnitTy& operator[](std::size_t pos)       noexcept { return _elems[pos]; }
        constexpr const UnitTy& operator[](std::size_t pos) const noexcept { return _elems[pos]; }

    public:
        template<std::size_t OtherDims, typename OtherUnitTy>
        constexpr explicit operator DerivedTy<OtherDims, OtherUnitTy>() const noexcept {
            static_assert(std::is_convertible_v<UnitTy, OtherUnitTy>, "No valid conversion between vector types!");
            auto convert_vector = [this]<std::size_t... I>(std::index_sequence<I...>) noexcept {
                return DerivedTy<OtherDims, OtherUnitTy>{static_cast<OtherUnitTy>(_elems[I])...};
            };
            constexpr std::size_t new_dims = OtherDims > Dims ? Dims : OtherDims;
            return convert_vector(std::make_index_sequence<new_dims>{});
        }
    };
}

namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    struct vector : vector_base<vector, Dims, UnitTy> {};


    template<typename UnitTy>
    struct vector<2, UnitTy> : vector_base<vector, 2, UnitTy> {
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept { return {x(), y()}; }

        constexpr explicit operator std::enable_if_t<std::is_convertible_v<UnitTy, std::int32_t>, VkOffset2D>() const noexcept { 
            return {static_cast<std::int32_t>(x()), static_cast<std::int32_t>(y())}; 
        }

        constexpr       UnitTy& x()       noexcept { return this->_elems[0]; }
        constexpr       UnitTy& y()       noexcept { return this->_elems[1]; }
        constexpr const UnitTy& x() const noexcept { return this->_elems[0]; }
        constexpr const UnitTy& y() const noexcept { return this->_elems[1]; }
    };

    template<typename UnitTy>
    struct vector<3, UnitTy> : vector_base<vector, 3, UnitTy> {
        constexpr explicit operator std::enable_if_t<std::is_convertible_v<UnitTy, std::int32_t>, VkOffset3D>() const noexcept { 
            return {static_cast<std::int32_t>(x()), static_cast<std::int32_t>(y()), static_cast<std::int32_t>(z())}; 
        }

        constexpr       UnitTy& x()       noexcept { return this->_elems[0]; }
        constexpr       UnitTy& y()       noexcept { return this->_elems[1]; }
        constexpr       UnitTy& z()       noexcept { return this->_elems[2]; }
        constexpr const UnitTy& x() const noexcept { return this->_elems[0]; }
        constexpr const UnitTy& y() const noexcept { return this->_elems[1]; }
        constexpr const UnitTy& z() const noexcept { return this->_elems[2]; }
    };
}

namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using vec = vector<Dims, UnitTy>;

    template<typename UnitTy> using vector2 = vector<2, UnitTy>;
    template<typename UnitTy> using vector3 = vector<3, UnitTy>;
    template<typename UnitTy> using vec2 = vector<2, UnitTy>;
    template<typename UnitTy> using vec3 = vector<3, UnitTy>;
}