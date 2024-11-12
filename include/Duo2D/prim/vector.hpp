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
    template<std::size_t Dims, typename UnitTy, bool HoldsSize>
    struct vector_traits {
        using vk_type = std::conditional_t<!HoldsSize,
            std::conditional_t<Dims == 2, VkOffset2D, VkOffset3D>, 
            std::conditional_t<Dims == 2, VkExtent2D, VkExtent3D>
        >;

        using vk_component_type = std::conditional_t<!HoldsSize,
            decltype(std::declval<VkOffset2D>().x), 
            decltype(std::declval<VkExtent2D>().height)
        >;
    };

    template<std::size_t Dims, typename UnitTy>
    struct vertex_traits {
        constexpr static VkFormat format = VK_FORMAT_UNDEFINED;
    };

    template<std::size_t Dims>
    concept Cartesian = Dims == 2 || Dims == 3;

    template<std::size_t Dims, typename T, bool HoldsSize>
    concept VkCompatibleType = Cartesian<Dims> && std::is_convertible_v<T, typename vector_traits<Dims, T, HoldsSize>::vk_component_type>; 
}


namespace d2d {
    template<std::size_t Dims, typename UnitTy, bool HoldsSize = false>
    struct vector {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
        //aggregate
        std::array<UnitTy, Dims> _elems;

        constexpr       UnitTy& operator[](std::size_t pos)       noexcept { return _elems[pos]; }
        constexpr const UnitTy& operator[](std::size_t pos) const noexcept { return _elems[pos]; }

    public:
        constexpr static VkFormat format = impl::vertex_traits<Dims, UnitTy>::format;
        using vk_type           = typename impl::vector_traits<Dims, UnitTy, HoldsSize>::vk_type;
        using vk_component_type = typename impl::vector_traits<Dims, UnitTy, HoldsSize>::vk_component_type;


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
        //TODO: Add more arithmetic operations (and use SIMD)
        template<bool OtherHoldsSize> constexpr vector& operator+=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) _elems[i] += rhs._elems[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator-=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) _elems[i] -= rhs._elems[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator*=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) _elems[i] *= rhs._elems[i]; return *this; }
        template<bool OtherHoldsSize> constexpr vector& operator/=(const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) _elems[i] /= rhs._elems[i]; return *this; }

        template<bool OtherHoldsSize> friend constexpr vector operator+(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs += rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator-(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs -= rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator*(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs *= rhs; }
        template<bool OtherHoldsSize> friend constexpr vector operator/(vector lhs, const vector<Dims, UnitTy, OtherHoldsSize>& rhs) noexcept { return lhs /= rhs; }


    public:
        constexpr explicit operator std::array<UnitTy, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<decltype(_elems)>()))() const noexcept { return std::tuple_cat(_elems); }
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept requires (Dims == 2) { return {_elems[0], _elems[1]}; }

    public:
        constexpr explicit operator vk_type() const noexcept requires (impl::VkCompatibleType<Dims, UnitTy, HoldsSize>) {
            return to<vk_type, vk_component_type>(std::make_index_sequence<Dims>{});
        }

        template<std::size_t OtherDims, typename OtherUnitTy, bool OtherHoldsSize> requires (std::is_convertible_v<UnitTy, OtherUnitTy>)
        constexpr explicit operator vector<OtherDims, OtherUnitTy, OtherHoldsSize>() const noexcept {
            constexpr std::size_t new_dims = OtherDims > Dims ? Dims : OtherDims;
            return to<vector<OtherDims, OtherUnitTy, OtherHoldsSize>, OtherUnitTy>(std::make_index_sequence<new_dims>{});
        }

    private:
        template<typename RetTy, typename CastTy, std::size_t... I>
        constexpr RetTy to(std::index_sequence<I...>) const noexcept { return RetTy{static_cast<CastTy>(_elems[I])...}; }
    };
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