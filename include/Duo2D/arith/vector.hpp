#pragma once
#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cmath>
#include <vulkan/vulkan_core.h>

#include "Duo2D/arith/axis.hpp"
#include "Duo2D/traits/vector_traits.hpp"


namespace d2d {
    template<std::size_t Dims, typename T, impl::vec_data_type HoldsData = impl::vec_data_type::point, std::uint8_t TransformFlags = 0>
    struct vector : public std::array<T, Dims> {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
    private:
        using clean_vector = vector<Dims, T, HoldsData, 0>;
        template<typename L, typename R, typename Op> using result_vector = impl::result_vector<Dims, L, R, Op, HoldsData, TransformFlags>;

    public:
        using vk_type           = typename impl::vector_traits<Dims, T, HoldsData, TransformFlags>::vk_type;
        using vk_component_type = typename impl::vector_traits<Dims, T, HoldsData, TransformFlags>::vk_component_type;
        constexpr static bool scalable     = impl::vector_traits<Dims, T, HoldsData, TransformFlags>::scalable;
        constexpr static bool rotatable    = impl::vector_traits<Dims, T, HoldsData, TransformFlags>::rotatable;
        constexpr static bool translatable = impl::vector_traits<Dims, T, HoldsData, TransformFlags>::translatable;

    public:
        using punned_integer_type = 
            std::conditional_t<sizeof(std::array<T, Dims>) * CHAR_BIT ==  8, std::uint8_t,
            std::conditional_t<sizeof(std::array<T, Dims>) * CHAR_BIT == 16, std::uint16_t,
            std::conditional_t<sizeof(std::array<T, Dims>) * CHAR_BIT == 32, std::uint32_t, 
            std::conditional_t<sizeof(std::array<T, Dims>) * CHAR_BIT == 64, std::uint64_t,
        void>>>>;
        constexpr static bool punnable = !std::is_same_v<punned_integer_type, void>;

    public:
        constexpr       T& x()       noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[0]; }
        constexpr const T& x() const noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[0]; }
        constexpr       T& y()       noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[1]; }
        constexpr const T& y() const noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[1]; }
        constexpr       T& z()       noexcept requires ((Dims == 3 || Dims == 4) && HoldsData == impl::vec_data_type::point) { return (*this)[2]; }
        constexpr const T& z() const noexcept requires ((Dims == 3 || Dims == 4) && HoldsData == impl::vec_data_type::point) { return (*this)[2]; }
        constexpr       T& w()       noexcept requires (Dims == 4 && HoldsData == impl::vec_data_type::point) { return (*this)[3]; }
        constexpr const T& w() const noexcept requires (Dims == 4 && HoldsData == impl::vec_data_type::point) { return (*this)[3]; }

    public:
        constexpr       T& width()        noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[0]; }
        constexpr const T& width()  const noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[0]; }
        constexpr       T& height()       noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[1]; }
        constexpr const T& height() const noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[1]; }
        constexpr       T& depth()        noexcept requires (Dims == 3 && HoldsData == impl::vec_data_type::size) { return (*this)[2]; }
        constexpr const T& depth()  const noexcept requires (Dims == 3 && HoldsData == impl::vec_data_type::size) { return (*this)[2]; }

    public:
        constexpr vector& invert_axis(axis a) noexcept requires impl::within_graphical_coordinates<Dims> { (*this)[static_cast<std::size_t>(a)] = -(*this)[static_cast<std::size_t>(a)]; return *this; }
        constexpr vector with_inverted_axis(axis a) const noexcept requires impl::within_graphical_coordinates<Dims> { 
            vector ret = *this;
            ret.invert_axis(a);
            return ret;
        }

    
    public:
        //TODO: SIMD (probably just needs a target_clones)
        template<typename U, impl::vec_data_type OtherHoldsData> constexpr vector& operator+=(const vector<Dims, U, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] += rhs[i]; return *this; }
        template<typename U, impl::vec_data_type OtherHoldsData> constexpr vector& operator-=(const vector<Dims, U, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] -= rhs[i]; return *this; }
        template<typename U, impl::vec_data_type OtherHoldsData> constexpr vector& operator*=(const vector<Dims, U, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] *= rhs[i]; return *this; }
        template<typename U, impl::vec_data_type OtherHoldsData> constexpr vector& operator/=(const vector<Dims, U, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] /= rhs[i]; return *this; }

        template<typename U, impl::vec_data_type OtherHoldsData> friend constexpr result_vector<T, U, std::plus<>      > operator+(vector lhs, const vector<Dims, U, OtherHoldsData>& rhs) noexcept { result_vector<T, U, std::plus<>      > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] + rhs[i]); return ret; }
        template<typename U, impl::vec_data_type OtherHoldsData> friend constexpr result_vector<T, U, std::minus<>     > operator-(vector lhs, const vector<Dims, U, OtherHoldsData>& rhs) noexcept { result_vector<T, U, std::minus<>     > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] - rhs[i]); return ret; }
        template<typename U, impl::vec_data_type OtherHoldsData> friend constexpr result_vector<T, U, std::multiplies<>> operator*(vector lhs, const vector<Dims, U, OtherHoldsData>& rhs) noexcept { result_vector<T, U, std::multiplies<>> ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] * rhs[i]); return ret; }
        template<typename U, impl::vec_data_type OtherHoldsData> friend constexpr result_vector<T, U, std::divides<>   > operator/(vector lhs, const vector<Dims, U, OtherHoldsData>& rhs) noexcept { result_vector<T, U, std::divides<>   > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] / rhs[i]); return ret; }
    public:
        template<impl::non_vector U> constexpr vector& operator+=(const U& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] += rhs; return *this; }
        template<impl::non_vector U> constexpr vector& operator-=(const U& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] -= rhs; return *this; }
        template<impl::non_vector U> constexpr vector& operator*=(const U& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] *= rhs; return *this; }
        template<impl::non_vector U> constexpr vector& operator/=(const U& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] /= rhs; return *this; }

        //TODO expression templates
        template<impl::non_vector U> friend constexpr result_vector<T, U, std::plus<>      > operator+(vector lhs, const U& rhs) noexcept { result_vector<T, U, std::plus<>      > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] + rhs   ); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<T, U, std::minus<>     > operator-(vector lhs, const U& rhs) noexcept { result_vector<T, U, std::minus<>     > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] - rhs   ); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<T, U, std::multiplies<>> operator*(vector lhs, const U& rhs) noexcept { result_vector<T, U, std::multiplies<>> ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] * rhs   ); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<T, U, std::divides<>   > operator/(vector lhs, const U& rhs) noexcept { result_vector<T, U, std::divides<>   > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] / rhs   ); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<U, T, std::plus<>      > operator+(U lhs, const vector& rhs) noexcept { result_vector<U, T, std::plus<>      > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    + rhs[i]); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<U, T, std::minus<>     > operator-(U lhs, const vector& rhs) noexcept { result_vector<U, T, std::minus<>     > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    - rhs[i]); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<U, T, std::multiplies<>> operator*(U lhs, const vector& rhs) noexcept { result_vector<U, T, std::multiplies<>> ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    * rhs[i]); return ret;  }
        template<impl::non_vector U> friend constexpr result_vector<U, T, std::divides<>   > operator/(U lhs, const vector& rhs) noexcept { result_vector<U, T, std::divides<>   > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    / rhs[i]); return ret;  }

        constexpr vector operator-() const noexcept { vector ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = -(*this)[i]; return ret; } 

    public:
        ///redundant
        //template<impl::vec_data_type OtherHoldsData>
        //constexpr explicit operator vector<Dims, T, OtherHoldsData, TransformFlags>() const noexcept { return std::bit_cast<vector<Dims, T, OtherHoldsData, TransformFlags>>(*this); }
        constexpr explicit operator clean_vector() const noexcept { return std::bit_cast<clean_vector>(*this); }

    public:
        //constexpr explicit operator std::array<T, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<std::array<T, Dims>>()))() const noexcept { return std::tuple_cat(*this); }
        constexpr explicit operator std::pair<T, T>() const noexcept requires (Dims == 2) { return {(*this)[0], (*this)[1]}; }

    public:
        constexpr explicit operator vk_type() const noexcept requires (impl::vk_vector_compatible<Dims, T, HoldsData, TransformFlags>) {
            return to<vk_type, vk_component_type>(std::make_index_sequence<Dims>{});
        }

        template<std::size_t OtherDims, typename OtherUnitTy, impl::vec_data_type OtherHoldsData> requires (std::is_convertible_v<T, OtherUnitTy>)
        constexpr explicit operator vector<OtherDims, OtherUnitTy, OtherHoldsData>() const noexcept {
            constexpr std::size_t new_dims = OtherDims > Dims ? Dims : OtherDims;
            return to<vector<OtherDims, OtherUnitTy, OtherHoldsData>, OtherUnitTy>(std::make_index_sequence<new_dims>{});
        }

    private:
        template<typename RetTy, typename CastTy, std::size_t... I>
        constexpr RetTy to(std::index_sequence<I...>) const noexcept { return RetTy{static_cast<CastTy>((*this)[I])...}; }
    };
}

namespace d2d {
    //TODO target_clones (except maybe dot, which might need manual SIMD for _mm_dp_ps)
    template<std::size_t N, typename LeftT, typename RightT> 
    constexpr typename impl::result_vector<N, LeftT, RightT, std::multiplies<>>::value_type dot(vector<N, LeftT> lhs, const vector<N, RightT>& rhs) noexcept { 
        return []<typename VL, typename VR, std::size_t... I>(VL&& l, VR&& r, std::index_sequence<I...>){ 
            return ((std::forward<VL>(l)[I] * std::forward<VR>(r)[I]) + ...); 
        }(lhs, rhs, std::make_index_sequence<N>{});
    }

    template<std::size_t N, typename T, impl::vec_data_type HD, typename F> 
    constexpr vector<N, T, HD> normalized(vector<N, T, HD> v, F&& sqrt_fn) noexcept { 
        T len = static_cast<T>(std::forward<F>(sqrt_fn)(dot(v,v)));
        auto do_norm = [&len, &v]<std::size_t... I>(std::index_sequence<I...>){ return vector<N, T, HD>{(v[I]/len)...}; }; 
        return do_norm(std::make_index_sequence<N>{});
    }
    template<std::size_t N, typename T, impl::vec_data_type HD> 
    vector<N, T, HD> normalized(vector<N, T, HD> v) noexcept { return normalized(v, static_cast<T(&)(T)>(std::sqrt)); }
        
    template<typename LeftT, typename RightT> 
    constexpr typename impl::result_vector<2, LeftT, RightT, std::multiplies<>>::value_type cross(vector<2, LeftT> lhs, const vector<2, RightT>& rhs) noexcept { 
        return lhs[0] * rhs[1] - lhs[1] * rhs[0];
    }
    template<typename T, typename U> 
    constexpr impl::result_vector<3, T, U, std::multiplies<>> cross(vector<3, T> lhs, const vector<3, U>& rhs) noexcept { 
        return {(lhs[1] * rhs[2] - lhs[2] * rhs[1]), (lhs[2] * rhs[0] - lhs[0] * rhs[2]), (lhs[0] * rhs[1] - lhs[1] * rhs[0])};
    }
}


//TODO?: make a generalized std::hash specialization and make this a custom hash type
template<std::size_t Dims, typename T, d2d::impl::vec_data_type HoldsData, std::uint8_t TransformFlags> requires (sizeof(d2d::vector<Dims, T, HoldsData, TransformFlags>) <= sizeof(std::size_t) && d2d::vector<Dims, T, HoldsData, TransformFlags>::punnable)
struct std::hash<d2d::vector<Dims, T, HoldsData, TransformFlags>> {
    constexpr std::size_t operator()(d2d::vector<Dims, T, HoldsData, TransformFlags> const& vec) const noexcept {
        return static_cast<std::size_t>(std::bit_cast<typename d2d::vector<Dims, T, HoldsData, TransformFlags>::punned_integer_type>(vec));
    }
};


namespace d2d {
    template<std::size_t Dims, typename T>
    using vec = vector<Dims, T>;

    template<typename T> using vector2 = vector<2, T>;
    template<typename T> using vector3 = vector<3, T>;
    template<typename T> using vector4 = vector<4, T>;
    template<typename T> using vec2 = vec<2, T>;
    template<typename T> using vec3 = vec<3, T>;
    template<typename T> using vec4 = vec<4, T>;
}

