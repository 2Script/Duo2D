#pragma once
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <array>
#include <utility>

#include "Duo2D/arith/axis.hpp"
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/traits/aggregate_traits.hpp"



namespace d2d {
    template<impl::arithmetic T>
    struct quaternion;
}

namespace d2d::impl {
    template<typename LeftT, typename RightT, typename Op> using result_quaternion = quaternion<decltype(std::declval<Op>()(std::declval<LeftT>(), std::declval<RightT>()))>;
}


namespace d2d {
    template<impl::arithmetic T>
    struct quaternion : public std::array<T, 4> {
    public:
        using punned_integer_type = 
            std::conditional_t<sizeof(std::array<T, 4>) * CHAR_BIT ==  8, std::uint8_t,
            std::conditional_t<sizeof(std::array<T, 4>) * CHAR_BIT == 16, std::uint16_t,
            std::conditional_t<sizeof(std::array<T, 4>) * CHAR_BIT == 32, std::uint32_t, 
            std::conditional_t<sizeof(std::array<T, 4>) * CHAR_BIT == 64, std::uint64_t,
        void>>>>;
        constexpr static bool punnable = !std::is_same_v<punned_integer_type, void>;

    public:
        constexpr       T& s()       noexcept { return (*this)[0]; }
        constexpr const T& s() const noexcept { return (*this)[0]; }
        constexpr       T& x()       noexcept { return (*this)[1]; }
        constexpr const T& x() const noexcept { return (*this)[1]; }
        constexpr       T& y()       noexcept { return (*this)[2]; }
        constexpr const T& y() const noexcept { return (*this)[2]; }
        constexpr       T& z()       noexcept { return (*this)[3]; }
        constexpr const T& z() const noexcept { return (*this)[3]; }


    public:
        constexpr T scalar_component() const noexcept { return (*this)[0]; }
        constexpr vec3<T> vector_component() const noexcept { return {(*this)[1], (*this)[2], (*this)[3]}; }

    public:
        template<impl::arithmetic U>
        constexpr explicit operator quaternion<U>() const noexcept { return {static_cast<U>((*this)[0]), static_cast<U>((*this)[1]), static_cast<U>((*this)[2]), static_cast<U>((*this)[3])}; }

    
    private:
        template<impl::arithmetic U> constexpr quaternion& mul(const quaternion<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& div(const quaternion<U>& rhs) noexcept;
    
    public:
        //TODO: SIMD (probably just needs a target_clones)
        template<impl::arithmetic U> constexpr quaternion& operator+=(const quaternion<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator-=(const quaternion<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator*=(const quaternion<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator/=(const quaternion<U>& rhs) noexcept;

    public:
        //TODO: SIMD (probably just needs a target_clones)
        template<impl::arithmetic U> constexpr quaternion& operator+=(const vec3<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator-=(const vec3<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator*=(const vec3<U>& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator/=(const vec3<U>& rhs) noexcept;

    public:
        template<impl::arithmetic U> constexpr quaternion& operator+=(const U& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator-=(const U& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator*=(const U& rhs) noexcept;
        template<impl::arithmetic U> constexpr quaternion& operator/=(const U& rhs) noexcept;

    public:
        constexpr quaternion operator-() const noexcept;
    };
}

namespace d2d {
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const quaternion<R>& rhs) noexcept;

    //TODO expression templates
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const R& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const R& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const R& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const R& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(L lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(L lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(L lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(L lhs, const quaternion<R>& rhs) noexcept;

    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const vec3<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const vec3<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const vec3<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const vec3<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(vec3<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(vec3<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(vec3<L> lhs, const quaternion<R>& rhs) noexcept;
    template<impl::arithmetic L, impl::arithmetic R> constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(vec3<L> lhs, const quaternion<R>& rhs) noexcept;
}


namespace d2d {
    template<typename T, typename F> 
    constexpr quaternion<T> normalized(quaternion<T> q, F&& sqrt_fn) noexcept { 
        T len = static_cast<T>(std::forward<F>(sqrt_fn)(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]));
        return quaternion<T>{q[0]/len, q[1]/len, q[2]/len, q[3]/len};
    }
}

namespace d2d {
    template<impl::arithmetic T, typename A, std::regular_invocable<A> SinFn, std::regular_invocable<A> CosFn>
    constexpr quaternion<T> to_quaternion(axis positive_rotation_axis, A rotation_angle, SinFn&& sin_fn, CosFn&& cos_fn) noexcept;
    
    template<impl::arithmetic T, typename A, std::regular_invocable<A> SinFn, std::regular_invocable<A> CosFn>
    constexpr quaternion<T> to_quaternion(A roll, A pitch, A yaw, SinFn&& sin_fn, CosFn&& cos_fn) noexcept;

    template<impl::arithmetic T>
    constexpr quaternion<T> to_quaternion(T s, vec3<T> xyz) noexcept;
    

    template<std::size_t N, impl::arithmetic T>
    constexpr matrix<4, 4, T> to_matrix(quaternion<T> q) noexcept requires (N == 4);

    template<std::size_t N, impl::arithmetic T>
    constexpr matrix<3, 3, T> to_matrix(quaternion<T> q) noexcept requires (N == 3);
}


namespace d2d {
    template<typename T>
    using quat = quaternion<T>;

    using quatf = quat<float>;
    using quatd = quat<double>;
}


#include "Duo2D/arith/quaternion.inl"
