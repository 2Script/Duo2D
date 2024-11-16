#pragma once
#include "Duo2D/arith/matrix.hpp"
#include <type_traits>
#include <utility>


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


