#pragma once
#include "Duo2D/arith/matrix.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace d2d::impl {
    enum transform_flags {
        scale = 0b001, rotate = 0b010, translate = 0b100
    };

    template<std::size_t Dims, typename UnitTy, bool HoldsSize, std::uint8_t TransformFlags>
    struct vector_traits {
        using vk_type = std::conditional_t<!HoldsSize,
            std::conditional_t<Dims == 2, VkOffset2D, VkOffset3D>, 
            std::conditional_t<Dims == 2, VkExtent2D, VkExtent3D>
        >;

        using vk_component_type = std::conditional_t<!HoldsSize,
            decltype(std::declval<VkOffset2D>().x), 
            decltype(std::declval<VkExtent2D>().height)
        >;

        constexpr static bool scalable     = !(TransformFlags & (rotate | translate));
        constexpr static bool rotatable    = !(TransformFlags & translate);
        constexpr static bool translatable = true;
    };

    template<std::size_t Dims, typename UnitTy>
    struct vertex_traits {
        constexpr static VkFormat format = VK_FORMAT_UNDEFINED;
    };

    template<std::size_t Dims>
    concept Cartesian = Dims == 2 || Dims == 3;

    template<std::size_t Dims>
    concept Graphical = Cartesian<Dims> || Dims == 4;

    template<std::size_t Dims, typename T, bool HoldsSize, std::uint8_t TransformFlags>
    concept VkCompatibleType = Cartesian<Dims> && std::is_convertible_v<T, typename vector_traits<Dims, T, HoldsSize, TransformFlags>::vk_component_type>; 
}


namespace d2d {
    template<std::size_t Dims, typename UnitTy, bool HoldsSize = false, std::uint8_t TransformFlags = 0>
    struct vector : public std::array<UnitTy, Dims> {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
    private:
        using clean_vector = vector<Dims, UnitTy, HoldsSize, 0>;

    public:
        constexpr static VkFormat format = impl::vertex_traits<Dims, UnitTy>::format;
        using vk_type           = typename impl::vector_traits<Dims, UnitTy, HoldsSize, TransformFlags>::vk_type;
        using vk_component_type = typename impl::vector_traits<Dims, UnitTy, HoldsSize, TransformFlags>::vk_component_type;
        constexpr static bool scalable     = impl::vector_traits<Dims, UnitTy, HoldsSize, TransformFlags>::scalable;
        constexpr static bool rotatable    = impl::vector_traits<Dims, UnitTy, HoldsSize, TransformFlags>::rotatable;
        constexpr static bool translatable = impl::vector_traits<Dims, UnitTy, HoldsSize, TransformFlags>::translatable;


    public:
        constexpr       UnitTy& x()       noexcept requires (impl::Graphical<Dims> && !HoldsSize) { return (*this)[0]; }
        constexpr const UnitTy& x() const noexcept requires (impl::Graphical<Dims> && !HoldsSize) { return (*this)[0]; }
        constexpr       UnitTy& y()       noexcept requires (impl::Graphical<Dims> && !HoldsSize) { return (*this)[1]; }
        constexpr const UnitTy& y() const noexcept requires (impl::Graphical<Dims> && !HoldsSize) { return (*this)[1]; }
        constexpr       UnitTy& z()       noexcept requires ((Dims == 3 || Dims == 4) && !HoldsSize) { return (*this)[2]; }
        constexpr const UnitTy& z() const noexcept requires ((Dims == 3 || Dims == 4) && !HoldsSize) { return (*this)[2]; }
        constexpr       UnitTy& w()       noexcept requires (Dims == 4 && !HoldsSize) { return (*this)[3]; }
        constexpr const UnitTy& w() const noexcept requires (Dims == 4 && !HoldsSize) { return (*this)[3]; }

    public:
        constexpr       UnitTy& width()        noexcept requires (impl::Graphical<Dims> && HoldsSize) { return (*this)[0]; }
        constexpr const UnitTy& width()  const noexcept requires (impl::Graphical<Dims> && HoldsSize) { return (*this)[0]; }
        constexpr       UnitTy& height()       noexcept requires (impl::Graphical<Dims> && HoldsSize) { return (*this)[1]; }
        constexpr const UnitTy& height() const noexcept requires (impl::Graphical<Dims> && HoldsSize) { return (*this)[1]; }
        constexpr       UnitTy& depth()        noexcept requires (Dims == 3 && HoldsSize) { return (*this)[2]; }
        constexpr const UnitTy& depth()  const noexcept requires (Dims == 3 && HoldsSize) { return (*this)[2]; }

    
    public:
        //TODO: Add more arithmetic operations (and use SIMD - probably just needs a target_clones)
        template<bool OtherHoldsSize> constexpr vector& operator+=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] += rhs[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator-=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] -= rhs[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator*=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] *= rhs[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator/=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] /= rhs[i]; return *this; }

        template<bool OtherHoldsSize> friend constexpr vector operator+(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs += rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator-(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs -= rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator*(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs *= rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator/(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs /= rhs; }

    public:
        constexpr explicit operator clean_vector() const noexcept { return {*this}; }


    public:
        //constexpr explicit operator std::array<UnitTy, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<std::array<UnitTy, Dims>>()))() const noexcept { return std::tuple_cat(*this); }
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept requires (Dims == 2) { return {(*this)[0], (*this)[1]}; }

    public:
        constexpr explicit operator vk_type() const noexcept requires (impl::VkCompatibleType<Dims, UnitTy, HoldsSize, TransformFlags>) {
            return to<vk_type, vk_component_type>(std::make_index_sequence<Dims>{});
        }

        template<std::size_t OtherDims, typename OtherUnitTy, bool OtherHoldsSize> requires (std::is_convertible_v<UnitTy, OtherUnitTy>)
        constexpr explicit operator vector<OtherDims, OtherUnitTy, OtherHoldsSize>() const noexcept {
            constexpr std::size_t new_dims = OtherDims > Dims ? Dims : OtherDims;
            return to<vector<OtherDims, OtherUnitTy, OtherHoldsSize>, OtherUnitTy>(std::make_index_sequence<new_dims>{});
        }

    private:
        template<typename RetTy, typename CastTy, std::size_t... I>
        constexpr RetTy to(std::index_sequence<I...>) const noexcept { return RetTy{static_cast<CastTy>((*this)[I])...}; }
    };
}

//matrix/vector transformations
namespace d2d {
    template<std::size_t N, typename T>
    class scale {
        template<std::size_t Dims, std::uint8_t Flags> using scaled_vector = vector<Dims, T, false, Flags | impl::transform_flags::scale>;
        template<std::size_t Dims, std::uint8_t Flags> using source_vector = vector<Dims, T, false, Flags>;

    public:
        vector<N, T> scale_by;

        template<std::uint8_t Flags>
        constexpr scaled_vector<N, Flags> operator()(source_vector<N, Flags> src_vec) const noexcept
        requires (impl::Cartesian<N> && source_vector<N + 1, Flags>::scalable) {
            return {src_vec * scale_by}; 
        }
        template<std::uint8_t Flags>
        constexpr scaled_vector<N + 1, Flags> operator()(source_vector<N + 1, Flags> src_vec) const noexcept
        requires (N + 1 >= 4 && source_vector<N + 1, Flags>::scalable) {
            return { matrix<N + 1, N + 1, T>::scaling(scale_by) * src_vec };
        }
    };

    template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<A>
    class rotate {
        template<std::size_t N, typename T, std::uint8_t Flags> using rotated_vector = vector<N, T, false, Flags | impl::transform_flags::rotate>;
        template<std::size_t N, typename T, std::uint8_t Flags> using source_vector  = vector<N, T, false, Flags>;

    public:
        A angle;
        FS& sin_fn;
        FC& cos_fn;
        axis rotate_axis = axis::x;

        template<std::size_t N, typename T, std::uint8_t Flags>
        constexpr rotated_vector<N, T, Flags> operator()(source_vector<N, T, Flags> src_vec) const noexcept
        requires (N == 2 && source_vector<N, T, Flags>::rotatable) {
            return { matrix<N, N, T>::rotating(angle, sin_fn, cos_fn) * src_vec };
        }
        template<std::size_t N, typename T, std::uint8_t Flags>
        constexpr rotated_vector<N, T, Flags> operator()(source_vector<N, T, Flags> src_vec) const noexcept
        requires (N >= 3 && source_vector<N, T, Flags>::rotatable) {
            return { matrix<N, N, T>::rotating(angle, sin_fn, cos_fn, rotate_axis) * src_vec };
        }
    };

    template<std::size_t N, typename T>
    class translate {
        template<std::size_t Dims, std::uint8_t Flags> using translated_vector = vector<Dims, T, false, Flags | impl::transform_flags::translate>;
        template<std::size_t Dims, std::uint8_t Flags> using source_vector     = vector<Dims, T, false, Flags>;

    public:
        vector<N, T> translate_by;

        template<std::uint8_t Flags>
        constexpr translated_vector<N, Flags> operator()(source_vector<N, Flags> src_vec) const noexcept 
        requires (impl::Cartesian<N> && source_vector<N + 1, Flags>::translatable) {
            return {src_vec + translate_by}; 
        }
        template<std::uint8_t Flags>
        constexpr translated_vector<N + 1, Flags> operator()(source_vector<N + 1, Flags> src_vec) const noexcept 
        requires (N + 1 >= 4 && source_vector<N + 1, Flags>::translatable) {
            return { matrix<N + 1, N + 1, T>::translating(translate_by) * src_vec };
        }
    };


    template<std::size_t N, typename T>
    scale(T (&&)[N]) -> scale<N, T>;
    template<typename T, typename... Args>
    scale(T head, Args... rest) -> scale<sizeof...(Args) + 1, T>;

    template<std::size_t N, typename T>
    translate(T (&&)[N]) -> translate<N, T>;
    template<typename T, typename... Args>
    translate(T head, Args... rest) -> translate<sizeof...(Args) + 1, T>;
}

//cumulative transform function
namespace d2d {
    namespace impl {
        template<typename Arg> struct scale_type { constexpr static bool value = requires (Arg a){ a.scale_by; }; };
        template<typename Arg> struct rotate_type { constexpr static bool value = requires (Arg a){ a.angle; }; };
        template<typename Arg> struct translate_type { constexpr static bool value = requires (Arg a){ a.translate_by; }; };
        
        //TODO make this work with perfect forwarding?
        template<template<typename> typename TransformType, std::size_t N, typename T, std::uint8_t Flags, typename Arg>
        constexpr decltype(auto) do_transform(vector<N, T, false, Flags> src_vec, Arg&& arg) { 
            if constexpr(TransformType<Arg>::value) return std::forward<Arg>(arg)(src_vec);
            else return src_vec;
        }
        template<template<typename> typename TransformType, std::size_t N, typename T, std::uint8_t Flags, typename First, typename... Args>
        constexpr decltype(auto) do_transform(vector<N, T, false, Flags> src_vec, First&& first, Args&&... args) {
            return do_transform<TransformType>(
                do_transform<TransformType>(src_vec, std::forward<First>(first)),
            std::forward<Args>(args)...);
        }
    }

    //TODO flatten/target_clones
    template<std::size_t Dims, typename UnitTy, typename... Args>
    constexpr vector<Dims, UnitTy> transform(vector<Dims, UnitTy> src_vec, Args&&... args) noexcept {
        auto scale_vec = impl::do_transform<impl::scale_type>(src_vec, std::forward<Args>(args)...);
        auto rotate_vec = impl::do_transform<impl::rotate_type>(scale_vec, std::forward<Args>(args)...);
        auto translate_vec = impl::do_transform<impl::translate_type>(rotate_vec, std::forward<Args>(args)...);
        return static_cast<vector<Dims, UnitTy>>(translate_vec);
    }
}


namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using vec = vector<Dims, UnitTy>;

    template<typename UnitTy> using vector2 = vector<2, UnitTy>;
    template<typename UnitTy> using vector3 = vector<3, UnitTy>;
    template<typename UnitTy> using vector4 = vector<4, UnitTy>;
    template<typename UnitTy> using vec2 = vec<2, UnitTy>;
    template<typename UnitTy> using vec3 = vec<3, UnitTy>;
    template<typename UnitTy> using vec4 = vec<4, UnitTy>;
}


namespace d2d::impl {
    template<> struct vertex_traits<1, float> { constexpr static VkFormat format = VK_FORMAT_R32_SFLOAT; };
    template<> struct vertex_traits<2, float> { constexpr static VkFormat format = VK_FORMAT_R32G32_SFLOAT; };
    template<> struct vertex_traits<3, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32_SFLOAT; };
    template<> struct vertex_traits<4, float> { constexpr static VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT; };
}