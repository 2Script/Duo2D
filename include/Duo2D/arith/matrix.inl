#pragma once
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/vector.hpp"
#include <type_traits>
#include <utility>


namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    consteval matrix<M, N, T> matrix<M, N, T>::identity() noexcept requires (M == N) {
        constexpr auto row = []<std::size_t I, std::size_t... Js>
        (std::integral_constant<std::size_t, I>, std::index_sequence<Js...>) {
            return std::array<T, N>{(I == Js ? 1 : 0)...};
        };
        constexpr auto expand = [row]<std::size_t... Is>(std::index_sequence<Is...>) {
            return matrix{(row(std::integral_constant<std::size_t, Is>{}, std::make_index_sequence<N>{}))...};
        };
        return expand(std::make_index_sequence<M>{});
    }

    template<std::size_t M, std::size_t N, typename T>
    constexpr matrix<M, N, T> matrix<M, N, T>::translating(std::array<T, N-1> translate_vec) noexcept requires (M == N) {
        matrix<M,N,T> m = matrix<M,N,T>::identity();
        for(std::size_t i = 0; i < (N - 1); ++i)
            m[i][N - 1] = translate_vec[i];
        return m;
    }

    template<std::size_t M, std::size_t N, typename T>
    constexpr matrix<M, N, T> matrix<M, N, T>::scaling(std::array<T, N-1> scale_vec) noexcept requires (M == N) {
        matrix<M,N,T> m = matrix<M,N,T>::identity();
        for(std::size_t i = 0; i < (N - 1); ++i)
            m[i][i] = scale_vec[i];
        return m;
    }


    template<std::size_t M, std::size_t N, typename T>
    template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<std::remove_cvref_t<A>>
    constexpr matrix<M, N, T> matrix<M, N, T>::rotating(A&& angle, FS&& sin_fn, FC&& cos_fn) noexcept requires (M == N && N == 2) {
        T s = static_cast<T>(std::forward<FS>(sin_fn)(std::forward<A>(angle)));
        T c = static_cast<T>(std::forward<FC>(cos_fn)(std::forward<A>(angle)));
        return matrix<M,N,T>{{std::array<T, N>{{c, -s}}, std::array<T, N>{{s, c}}}};
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<std::remove_cvref_t<A>>
    constexpr matrix<M, N, T> matrix<M, N, T>::rotating(A&& angle, axis rotate_axis, FS&& sin_fn, FC&& cos_fn) noexcept requires (M == N && (N >= 3)) {
        matrix<M,N,T> m = matrix<M,N,T>::identity();
        T s = static_cast<T>(std::forward<FS>(sin_fn)(std::forward<A>(angle)));
        T c = static_cast<T>(std::forward<FC>(cos_fn)(std::forward<A>(angle)));
        switch(rotate_axis){
        case axis::x:
            m[1][1] = c; m[1][2] = -s;
            m[2][1] = s; m[2][2] = c;
            break;
        case axis::y:
            m[0][0] = c;  m[0][2] = s;
            m[2][0] = -s; m[2][2] = c;
            break;
        case axis::z:
            m[0][0] = c; m[0][1] = -s;
            m[1][0] = s; m[1][1] = c;
            break;
        }
        return m;
    }
}


namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    template<typename Mat, typename Vec, std::size_t I, std::size_t... Js>
    constexpr T matrix<M,N,T>::mult_row(Mat&& m, Vec&& v, std::integral_constant<std::size_t, I>, std::index_sequence<Js...>) noexcept {
        return (((std::forward<Mat>(m)[I][Js])*(std::forward<Vec>(v)[Js])) + ...);
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename Mat, typename Vec, std::size_t... Is, std::size_t J>
    constexpr typename matrix<M,N,T>::column_type matrix<M,N,T>::mult(Mat&& m, Vec&& v, std::index_sequence<Is...>, std::integral_constant<std::size_t, J>) noexcept {
        return {mult_row(std::forward<Mat>(m), std::forward<Vec>(v), std::integral_constant<std::size_t, Is>{}, std::make_index_sequence<J>{})...};
    }
}

namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    template<typename Vec, typename Mat, std::size_t... Is, std::size_t J>
    constexpr T matrix<M,N,T>::mult_col(Vec&& v, Mat&& m, std::index_sequence<Is...>, std::integral_constant<std::size_t, J>) noexcept {
        return (((std::forward<Vec>(v)[Is])*(std::forward<Mat>(m)[Is][J])) + ...);
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename Vec, typename Mat, std::size_t I, std::size_t... Js>
    constexpr typename matrix<M,N,T>::row_type matrix<M,N,T>::mult(Vec&& v, Mat&& m, std::integral_constant<std::size_t, I>, std::index_sequence<Js...>) noexcept {
        return {mult_col(std::forward<Vec>(v), std::forward<Mat>(m), std::make_index_sequence<I>{}, std::integral_constant<std::size_t, Js>{})...};
    }
}


namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    template<typename MatA, typename MatB,  std::size_t I, std::size_t... Ks, std::size_t J>
    constexpr T matrix<M,N,T>::mult_mat(MatA&& m_a, MatB&& m_b, std::integral_constant<std::size_t, I>, std::index_sequence<Ks...>, std::integral_constant<std::size_t, J>) noexcept {
        return (((std::forward<MatA>(m_a)[I][Ks])*(std::forward<MatB>(m_b)[Ks][J])) + ...);
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename MatA, typename MatB, std::size_t I, std::size_t K, std::size_t... Js>
    constexpr typename matrix<M,N,T>::row_type matrix<M,N,T>::mult_mat_cols(MatA&& m_a, MatB&& m_b, std::integral_constant<std::size_t, I> i, std::integral_constant<std::size_t, K>, std::index_sequence<Js...>) noexcept {
        return {mult_mat(std::forward<MatA>(m_a), std::forward<MatB>(m_b), i, std::make_index_sequence<K>{}, std::integral_constant<std::size_t, Js>{})...};
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename MatA, typename MatB, std::size_t... Is, std::size_t K, std::size_t... Js>
    constexpr matrix<sizeof...(Is), sizeof...(Js), T> matrix<M,N,T>::mult(MatA&& m_a, MatB&& m_b, std::index_sequence<Is...>, std::integral_constant<std::size_t, K> k, std::index_sequence<Js...> js) noexcept {
        return {mult_mat_cols(std::forward<MatA>(m_a), std::forward<MatB>(m_b), std::integral_constant<std::size_t, Is>{}, k, js)...};
    }
}



namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    template<typename F>
    constexpr matrix<M, N, T> matrix<M, N, T>::looking_at(vector<3, T> eye, vector<3, T> center, axis up_axis, F&& sqrt_fn) noexcept requires (M == N && N >= 4) {
        vec3<T> f = normalized(center - eye, std::forward<F>(sqrt_fn)); 
        vec3<T> dir = {0,0,0};
        dir[static_cast<std::size_t>(up_axis)] = 1.f;
        vec3<T> s = normalized(cross(f, dir), std::forward<F>(sqrt_fn)); 
        vec3<T> t = cross(s, f);

        matrix<N, N, T> ret = matrix<N, N, T>::identity();
        for(std::size_t i = 0; i < 3; ++i) {
            ret[i][0] = s[i]; 
            ret[i][1] = t[i]; 
            ret[i][2] = -f[i]; 
        }
        ret[3][0] = dot(s, -eye);
        ret[3][1] = dot(t, -eye);
        ret[3][2] = dot(-f, -eye);
        return ret;
    }


    template<std::size_t M, std::size_t N, typename T>
    template<typename A, typename F>
    constexpr matrix<M, N, T> matrix<M, N, T>::perspective(A fov_angle, T screen_width, T screen_height, F&& tan_fn, T near_z, T far_z) noexcept requires (M == N && N == 4) {
        T focal = 1/static_cast<T>(std::forward<F>(tan_fn)(fov_angle / static_cast<A>(2)));
        T denom = (far_z - near_z);
        T a = near_z / denom;
        T b = (near_z * far_z) / denom;
        return {{{
            {{focal * screen_height / screen_width, 0, 0, 0 }},
            {{0, -focal, 0, 0 }},
            {{0, 0, a, -1 }},
            {{0, 0, b, 0 }}
        }}};
    }

    template<std::size_t M, std::size_t N, typename T>
    template<typename A, typename F>
    constexpr matrix<M, N, T> matrix<M, N, T>::perspective(A fov_angle, T screen_width, T screen_height, F&& tan_fn, T near_z) noexcept requires (M == N && N == 4) {
        T focal = 1/static_cast<T>(std::forward<F>(tan_fn)(fov_angle / static_cast<A>(2)));
        return {{{
            {{focal * screen_height / screen_width, 0, 0, 0 }},
            {{0, -focal, 0, 0 }},
            {{0, 0, 0, -1 }},
            {{0, 0, near_z, 0 }}
        }}};
    }


    template<std::size_t M, std::size_t N, typename T>
    constexpr matrix<M, N, T> matrix<M, N, T>::orthographic(T screen_width, T screen_height, T near_z, T far_z) noexcept requires (M == N && N == 4) {
        T denom = (far_z - near_z);
        return {{{
            {{2 / screen_width, 0, 0, 0 }},
            {{0, 2 / screen_height, 0, 0 }},
            {{0, 0, 1 / denom, far_z / denom }},
            {{0, 0, 0, 1 }}
        }}};
    }

    template<std::size_t M, std::size_t N, typename T>
    constexpr matrix<M, N, T> matrix<M, N, T>::orthographic(T screen_width, T screen_height) noexcept requires (M == N && N == 4) {
        return {{{
            {{2 / screen_width, 0, 0, 0 }},
            {{0, 2 / screen_height, 0, 0 }},
            {{0, 0, 0, 1 }},
            {{0, 0, 0, 1 }}
        }}};
    }
}



//matrix/vector transformations
namespace d2d {
    template<std::size_t N, typename T>
    class scale {
        template<std::size_t Dims, std::uint8_t Flags> using scaled_vector = vector<Dims, T, impl::vec_data_type::point, Flags | impl::transform_flags::scale>;
        template<std::size_t Dims, std::uint8_t Flags> using source_vector = vector<Dims, T, impl::vec_data_type::point, Flags>;

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
        template<std::size_t N, typename T, std::uint8_t Flags> using rotated_vector = vector<N, T, impl::vec_data_type::point, Flags | impl::transform_flags::rotate>;
        template<std::size_t N, typename T, std::uint8_t Flags> using source_vector  = vector<N, T, impl::vec_data_type::point, Flags>;

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
        template<std::size_t Dims, std::uint8_t Flags> using translated_vector = vector<Dims, T, impl::vec_data_type::point, Flags | impl::transform_flags::translate>;
        template<std::size_t Dims, std::uint8_t Flags> using source_vector     = vector<Dims, T, impl::vec_data_type::point, Flags>;

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
        constexpr decltype(auto) do_transform(vector<N, T, impl::vec_data_type::point, Flags> src_vec, Arg&& arg) { 
            if constexpr(TransformType<Arg>::value) return std::forward<Arg>(arg)(src_vec);
            else return src_vec;
        }
        template<template<typename> typename TransformType, std::size_t N, typename T, std::uint8_t Flags, typename First, typename... Args>
        constexpr decltype(auto) do_transform(vector<N, T, impl::vec_data_type::point, Flags> src_vec, First&& first, Args&&... args) {
            return do_transform<TransformType>(
                do_transform<TransformType>(src_vec, std::forward<First>(first)),
            std::forward<Args>(args)...);
        }
    }

    template<std::size_t Dims, typename UnitTy, typename... Args>
    constexpr point<Dims, UnitTy> transform(point<Dims, UnitTy> src_vec, Args&&... args) noexcept {
        auto scale_vec = impl::do_transform<impl::scale_type>(src_vec, std::forward<Args>(args)...);
        auto rotate_vec = impl::do_transform<impl::rotate_type>(scale_vec, std::forward<Args>(args)...);
        auto translate_vec = impl::do_transform<impl::translate_type>(rotate_vec, std::forward<Args>(args)...);
        return static_cast<point<Dims, UnitTy>>(translate_vec);
    }
}


