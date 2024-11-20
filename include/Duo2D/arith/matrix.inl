#pragma once
#include "Duo2D/arith/matrix.hpp"
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
            m[1][0] = s; m[2][2] = c;
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


