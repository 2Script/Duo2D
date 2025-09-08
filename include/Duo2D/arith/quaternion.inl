#pragma once
#include "Duo2D/arith/quaternion.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/traits/aggregate_traits.hpp"
#include <concepts>


namespace d2d {
    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::mul(const quaternion<U>& rhs) noexcept {
        vec3<T> lhs_vec = this->vector_component();
        vec3<T> rhs_vec = static_cast<vec3<T>>(rhs.vector_component());
        T lhs_scalar = this->scalar_component();
        T rhs_scalar = static_cast<T>(scalar_component());
        *this = to_quaternion(
            lhs_scalar * rhs_scalar - dot(lhs_vec, rhs_vec), 
            cross(lhs_vec, rhs_vec) + lhs_scalar * rhs_vec + rhs_scalar * lhs_vec
        );
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::div(const quaternion<U>& rhs) noexcept {
        T denom = static_cast<T>(rhs[0] * rhs[0] + rhs[1] * rhs[1] + rhs[2] * rhs[2] + rhs[3] * rhs[3]);
        (*this)[0] = static_cast<T>((*this)[0] * rhs[0] + (*this)[1] * rhs[1] + (*this)[2] * rhs[2] + (*this)[3] * rhs[3]) / denom;
        (*this)[1] = static_cast<T>((*this)[0] * rhs[1] - (*this)[1] * rhs[0] - (*this)[2] * rhs[3] - (*this)[3] * rhs[2]) / denom;
        (*this)[2] = static_cast<T>((*this)[0] * rhs[2] + (*this)[1] * rhs[3] - (*this)[2] * rhs[0] + (*this)[3] * rhs[1]) / denom;
        (*this)[3] = static_cast<T>((*this)[0] * rhs[3] - (*this)[1] * rhs[2] + (*this)[2] * rhs[1] - (*this)[3] * rhs[0]) / denom;
        return *this;
    }
}


namespace d2d {
    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator+=(const quaternion<U>& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] += rhs[i];
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator-=(const quaternion<U>& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] -= rhs[i];
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator*=(const quaternion<U>& rhs) noexcept {
        return mul(rhs);
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator/=(const quaternion<U>& rhs) noexcept {
        return div(rhs);
    }
}

namespace d2d {
    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator+=(const vec3<U>& rhs) noexcept {
        return *this += to_quaternion(static_cast<U>(0), rhs);
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator-=(const vec3<U>& rhs) noexcept {
        return *this -= to_quaternion(static_cast<U>(0), rhs);
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator*=(const vec3<U>& rhs) noexcept {
        return *this *= to_quaternion(static_cast<U>(0), rhs);
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U> 
    constexpr quaternion<T>& quaternion<T>::operator/=(const vec3<U>& rhs) noexcept {
        return *this /= to_quaternion(static_cast<U>(0), rhs);
    }
}

namespace d2d {
    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::operator+=(const U& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] += rhs;
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::operator-=(const U& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] -= rhs;
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::operator*=(const U& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] *= rhs;
        return *this;
    }

    template<impl::arithmetic T>
    template<impl::arithmetic U>
    constexpr quaternion<T>& quaternion<T>::operator/=(const U& rhs) noexcept {
        for(std::size_t i = 0; i < 4; ++i) (*this)[i] /= rhs;
        return *this;
    }

}

namespace d2d {
    template<impl::arithmetic T>
    constexpr quaternion<T> quaternion<T>::operator-() const noexcept { 
        quaternion ret; 
        for(std::size_t i = 1; i < 4; ++i) ret[i] = -(*this)[i]; 
        return ret;
    } 
}


namespace d2d {
    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::plus<>      > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] + rhs[i]);
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::minus<>     > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] - rhs[i]);
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::multiplies<>> ret = static_cast<impl::result_quaternion<L, R, std::multiplies<>>>(lhs);
        return ret *= rhs;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::divides<>   > ret = static_cast<impl::result_quaternion<L, R, std::multiplies<>>>(lhs);
        return ret *= rhs;
    }
}

namespace d2d {
    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const R& rhs) noexcept {
        impl::result_quaternion<L, R, std::plus<>      > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] + rhs   );
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const R& rhs) noexcept {
        impl::result_quaternion<L, R, std::minus<>     > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] - rhs   );
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const R& rhs) noexcept {
        impl::result_quaternion<L, R, std::multiplies<>> ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] * rhs   );
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const R& rhs) noexcept {
        impl::result_quaternion<L, R, std::divides<>   > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs[i] / rhs   );
        return ret;
    }

    
    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(L lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::plus<>      > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs    + rhs[i]);
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(L lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::minus<>     > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs    - rhs[i]);
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(L lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::multiplies<>> ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs    * rhs[i]);
        return ret;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(L lhs, const quaternion<R>& rhs) noexcept {
        impl::result_quaternion<L, R, std::divides<>   > ret;
        for(std::size_t i = 0; i < 4; ++i) ret[i] = (lhs    / rhs[i]);
        return ret;
    }
}

namespace d2d {
    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(quaternion<L> lhs, const vec3<R>& rhs) noexcept {
        return lhs + to_quaternion(static_cast<R>(0), rhs);
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(quaternion<L> lhs, const vec3<R>& rhs) noexcept {
        return lhs - to_quaternion(static_cast<R>(0), rhs);
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(quaternion<L> lhs, const vec3<R>& rhs) noexcept {
        return lhs * to_quaternion(static_cast<R>(0), rhs);
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(quaternion<L> lhs, const vec3<R>& rhs) noexcept {
        return lhs / to_quaternion(static_cast<R>(0), rhs);
    }

    
    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::plus<>      > operator+(vec3<L> lhs, const quaternion<R>& rhs) noexcept {
        return to_quaternion(static_cast<L>(0), lhs) + rhs;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::minus<>     > operator-(vec3<L> lhs, const quaternion<R>& rhs) noexcept {
        return to_quaternion(static_cast<L>(0), lhs) - rhs;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::multiplies<>> operator*(vec3<L> lhs, const quaternion<R>& rhs) noexcept {
        return to_quaternion(static_cast<L>(0), lhs) * rhs;
    }

    template<impl::arithmetic L, impl::arithmetic R>
    constexpr impl::result_quaternion<L, R, std::divides<>   > operator/(vec3<L> lhs, const quaternion<R>& rhs) noexcept {
        return to_quaternion(static_cast<L>(0), lhs) / rhs;
    }
}



namespace d2d {
    template<impl::arithmetic T, typename A, std::regular_invocable<A> SinFn, std::regular_invocable<A> CosFn>
    constexpr quaternion<T> to_quaternion(axis positive_rotation_axis, A rotation_angle, SinFn&& sin_fn, CosFn&& cos_fn) noexcept {
        vec3<T> axis_vec = {0,0,0};
        axis_vec[static_cast<std::size_t>(positive_rotation_axis)] = static_cast<T>(1);
        T sine = static_cast<T>(std::forward<SinFn>(sin_fn)(rotation_angle));
        return to_quaternion(static_cast<T>(std::forward<CosFn>(cos_fn)(rotation_angle)), axis_vec * sine);
    }

    template<impl::arithmetic T>
    constexpr quaternion<T> to_quaternion(T s, vec3<T> xyz) noexcept {
        return quaternion<T>{s, xyz[0], xyz[1], xyz[2]};
    }

    template<impl::arithmetic T, typename A, std::regular_invocable<A> SinFn, std::regular_invocable<A> CosFn>
    constexpr quaternion<T> to_quaternion(A roll, A pitch, A yaw, SinFn&& sin_fn, CosFn&& cos_fn) noexcept {
        T c_r = static_cast<T>(std::forward<CosFn>(cos_fn)(roll  / static_cast<A>(2)));
        T s_r = static_cast<T>(std::forward<SinFn>(sin_fn)(roll  / static_cast<A>(2)));
        T c_p = static_cast<T>(std::forward<CosFn>(cos_fn)(pitch / static_cast<A>(2)));
        T s_p = static_cast<T>(std::forward<SinFn>(sin_fn)(pitch / static_cast<A>(2)));
        T c_y = static_cast<T>(std::forward<CosFn>(cos_fn)(yaw   / static_cast<A>(2)));
        T s_y = static_cast<T>(std::forward<SinFn>(sin_fn)(yaw   / static_cast<A>(2)));

        //pitch and roll are swapped from the normal convention due to vulkan coordinate system
        return quaternion<T>{ 
            c_p * c_r * c_y + s_p * s_r * s_y,
            s_p * c_r * c_y - c_p * s_r * s_y,
            c_p * c_r * s_y - s_p * s_r * c_y,
            c_p * s_r * c_y + s_p * c_r * s_y,
        };
    }


    template<std::size_t N, impl::arithmetic T>
    constexpr matrix<4, 4, T> to_matrix(quaternion<T> q) noexcept requires (N == 4) {
        T s = q.s();
        T x = q.x();
        T y = q.y();
        T z = q.z();
        return {{{
            {{s * s + x * x - y * y - z * z, 2 * x * y - 2 * s * z,         2 * x * z + 2 * s * y,         0}},
            {{2 * x * y + 2 * s * z,         s * s - x * x + y * y - z * z, 2 * y * z - 2 * s * x,         0}},
            {{2 * x * z - 2 * s * y,         2 * y * z + 2 * s * x,         s * s - x * x - y * y + z * z, 0}},
            {{0, 0, 0, 1}},
        }}};
    }

    template<std::size_t N, impl::arithmetic T>
    constexpr matrix<3, 3, T> to_matrix(quaternion<T> q) noexcept requires (N == 3) {
        T s = q.s();
        T x = q.x();
        T y = q.y();
        T z = q.z();
        return {{{
            {{s * s + x * x - y * y - z * z, 2 * x * y - 2 * s * z,         2 * x * z + 2 * s * y,       }},
            {{2 * x * y + 2 * s * z,         s * s - x * x + y * y - z * z, 2 * y * z - 2 * s * x,       }},
            {{2 * x * z - 2 * s * y,         2 * y * z + 2 * s * x,         s * s - x * x - y * y + z * z}},
        }}};
    }
}