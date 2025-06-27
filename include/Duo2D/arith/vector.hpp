#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cmath>
#include <vulkan/vulkan_core.h>

#include "Duo2D/traits/vector_traits.hpp"


namespace d2d {
    template<std::size_t Dims, typename UnitTy, impl::vec_data_type HoldsData = impl::vec_data_type::none, std::uint8_t TransformFlags = 0>
    struct vector : public std::array<UnitTy, Dims> {
        static_assert(Dims > 0, "0-dimensional vector is not valid!");
    private:
        using clean_vector = vector<Dims, UnitTy, HoldsData, 0>;
        template<typename T, typename Op> using l_result_vector = vector<Dims, decltype(std::declval<Op>()(std::declval<UnitTy>(), std::declval<T>())), HoldsData, TransformFlags>;
        template<typename T, typename Op> using r_result_vector = vector<Dims, decltype(std::declval<Op>()(std::declval<T>(), std::declval<UnitTy>())), HoldsData, TransformFlags>;

    public:
        using vk_type           = typename impl::vector_traits<Dims, UnitTy, HoldsData, TransformFlags>::vk_type;
        using vk_component_type = typename impl::vector_traits<Dims, UnitTy, HoldsData, TransformFlags>::vk_component_type;
        constexpr static bool scalable     = impl::vector_traits<Dims, UnitTy, HoldsData, TransformFlags>::scalable;
        constexpr static bool rotatable    = impl::vector_traits<Dims, UnitTy, HoldsData, TransformFlags>::rotatable;
        constexpr static bool translatable = impl::vector_traits<Dims, UnitTy, HoldsData, TransformFlags>::translatable;


    public:
        constexpr       UnitTy& x()       noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[0]; }
        constexpr const UnitTy& x() const noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[0]; }
        constexpr       UnitTy& y()       noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[1]; }
        constexpr const UnitTy& y() const noexcept requires (impl::within_graphical_coordinates<Dims> && HoldsData == impl::vec_data_type::point) { return (*this)[1]; }
        constexpr       UnitTy& z()       noexcept requires ((Dims == 3 || Dims == 4) && HoldsData == impl::vec_data_type::point) { return (*this)[2]; }
        constexpr const UnitTy& z() const noexcept requires ((Dims == 3 || Dims == 4) && HoldsData == impl::vec_data_type::point) { return (*this)[2]; }
        constexpr       UnitTy& w()       noexcept requires (Dims == 4 && HoldsData == impl::vec_data_type::point) { return (*this)[3]; }
        constexpr const UnitTy& w() const noexcept requires (Dims == 4 && HoldsData == impl::vec_data_type::point) { return (*this)[3]; }

    public:
        constexpr       UnitTy& width()        noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[0]; }
        constexpr const UnitTy& width()  const noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[0]; }
        constexpr       UnitTy& height()       noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[1]; }
        constexpr const UnitTy& height() const noexcept requires (impl::within_cartesian_coordinates<Dims> && HoldsData == impl::vec_data_type::size) { return (*this)[1]; }
        constexpr       UnitTy& depth()        noexcept requires (Dims == 3 && HoldsData == impl::vec_data_type::size) { return (*this)[2]; }
        constexpr const UnitTy& depth()  const noexcept requires (Dims == 3 && HoldsData == impl::vec_data_type::size) { return (*this)[2]; }

    
    public:
        //TODO: SIMD (probably just needs a target_clones)
        template<impl::vec_data_type OtherHoldsData> constexpr vector& operator+=(const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] += rhs[i]; return *this; }
        template<impl::vec_data_type OtherHoldsData> constexpr vector& operator-=(const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] -= rhs[i]; return *this; }
        template<impl::vec_data_type OtherHoldsData> constexpr vector& operator*=(const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] *= rhs[i]; return *this; }
        template<impl::vec_data_type OtherHoldsData> constexpr vector& operator/=(const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] /= rhs[i]; return *this; }

        template<impl::vec_data_type OtherHoldsData> friend constexpr vector operator+(vector lhs, const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { return lhs += rhs; }
        template<impl::vec_data_type OtherHoldsData> friend constexpr vector operator-(vector lhs, const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { return lhs -= rhs; }
        template<impl::vec_data_type OtherHoldsData> friend constexpr vector operator*(vector lhs, const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { return lhs *= rhs; }
        template<impl::vec_data_type OtherHoldsData> friend constexpr vector operator/(vector lhs, const vector<Dims, UnitTy, OtherHoldsData>& rhs) noexcept { return lhs /= rhs; }
    public:
        template<impl::non_vector T> constexpr vector& operator+=(const T& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] += rhs; return *this; }
        template<impl::non_vector T> constexpr vector& operator-=(const T& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] -= rhs; return *this; }
        template<impl::non_vector T> constexpr vector& operator*=(const T& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] *= rhs; return *this; }
        template<impl::non_vector T> constexpr vector& operator/=(const T& rhs) noexcept { for(std::size_t i = 0; i < Dims; ++i) (*this)[i] /= rhs; return *this; }

        template<impl::non_vector T> friend constexpr l_result_vector<T, std::plus<>      > operator+(vector lhs, const T& rhs) noexcept { l_result_vector<T, std::plus<>      > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] + rhs   ); return ret;  }
        template<impl::non_vector T> friend constexpr l_result_vector<T, std::minus<>     > operator-(vector lhs, const T& rhs) noexcept { l_result_vector<T, std::minus<>     > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] - rhs   ); return ret;  }
        template<impl::non_vector T> friend constexpr l_result_vector<T, std::multiplies<>> operator*(vector lhs, const T& rhs) noexcept { l_result_vector<T, std::multiplies<>> ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] * rhs   ); return ret;  }
        template<impl::non_vector T> friend constexpr l_result_vector<T, std::divides<>   > operator/(vector lhs, const T& rhs) noexcept { l_result_vector<T, std::divides<>   > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs[i] / rhs   ); return ret;  }
        template<impl::non_vector T> friend constexpr r_result_vector<T, std::plus<>      > operator+(T lhs, const vector& rhs) noexcept { r_result_vector<T, std::plus<>      > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    + rhs[i]); return ret;  }
        template<impl::non_vector T> friend constexpr r_result_vector<T, std::minus<>     > operator-(T lhs, const vector& rhs) noexcept { r_result_vector<T, std::minus<>     > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    - rhs[i]); return ret;  }
        template<impl::non_vector T> friend constexpr r_result_vector<T, std::multiplies<>> operator*(T lhs, const vector& rhs) noexcept { r_result_vector<T, std::multiplies<>> ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    * rhs[i]); return ret;  }
        template<impl::non_vector T> friend constexpr r_result_vector<T, std::divides<>   > operator/(T lhs, const vector& rhs) noexcept { r_result_vector<T, std::divides<>   > ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = (lhs    / rhs[i]); return ret;  }

        constexpr vector operator-() const noexcept { vector ret; for(std::size_t i = 0; i < Dims; ++i) ret[i] = -(*this)[i]; return ret; } 

    public:
        constexpr explicit operator clean_vector() const noexcept { return {*this}; }

    public:
        //constexpr explicit operator std::array<UnitTy, Dims>() const noexcept { return _elems; }
        constexpr explicit operator decltype(std::tuple_cat(std::declval<std::array<UnitTy, Dims>>()))() const noexcept { return std::tuple_cat(*this); }
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept requires (Dims == 2) { return {(*this)[0], (*this)[1]}; }

    public:
        constexpr explicit operator vk_type() const noexcept requires (impl::vk_vector_compatible<Dims, UnitTy, HoldsData, TransformFlags>) {
            return to<vk_type, vk_component_type>(std::make_index_sequence<Dims>{});
        }

        template<std::size_t OtherDims, typename OtherUnitTy, impl::vec_data_type OtherHoldsData> requires (std::is_convertible_v<UnitTy, OtherUnitTy>)
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
    template<std::size_t N, typename T, impl::vec_data_type HD> 
    constexpr T dot(vector<N, T, HD> lhs, const vector<N, T, HD>& rhs) noexcept { 
        auto do_dot = [&lhs, &rhs]<std::size_t... I>(std::index_sequence<I...>){ return ((lhs[I] * rhs[I]) + ...); }; 
        return do_dot(std::make_index_sequence<N>{});
    }

    template<std::size_t N, typename T, impl::vec_data_type HD, typename F> 
    constexpr vector<N, T, HD> normalized(vector<N, T, HD> v, F&& sqrt_fn) noexcept { 
        T len = static_cast<T>(std::forward<F>(sqrt_fn)(dot(v,v)));
        auto do_norm = [&len, &v]<std::size_t... I>(std::index_sequence<I...>){ return vector<N, T, HD>{(v[I]/len)...}; }; 
        return do_norm(std::make_index_sequence<N>{});
    }
    template<std::size_t N, typename T, impl::vec_data_type HD> 
    vector<N, T, HD> normalized(vector<N, T, HD> v) noexcept { return normalized(v, static_cast<T(&)(T)>(std::sqrt)); }
        
    template<typename T, impl::vec_data_type HD> 
    constexpr T cross(vector<2, T, HD> lhs, const vector<2, T, HD>& rhs) noexcept { 
        return lhs[0] * rhs[1] - lhs[1] * rhs[0];
    }
    template<typename T, impl::vec_data_type HD> 
    constexpr vector<3, T, HD> cross(vector<3, T, HD> lhs, const vector<3, T, HD>& rhs) noexcept { 
        return {(lhs[1] * rhs[2] - lhs[2] * rhs[1]), (lhs[2] * rhs[0] - lhs[0] * rhs[2]), (lhs[0] * rhs[1] - lhs[1] * rhs[0])};
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

