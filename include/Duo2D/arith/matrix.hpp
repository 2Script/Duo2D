#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <streamline/metaprogramming/constant.hpp>

#include "Duo2D/arith/point.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/arith/axis.hpp"


namespace d2d {
    
    template<std::size_t M, std::size_t N, typename T>
    struct matrix : public std::array<std::array<T, N>, M> {
        constexpr static std::size_t rows = M;
        constexpr static std::size_t columns = N;
        using array_type = std::array<std::array<T, columns>, rows>;
        using row_type = std::array<T, rows>;
        using column_type = std::array<T, columns>;
        using flattened_type = std::array<T, rows * columns>;

    public:
        constexpr std::size_t size() const noexcept { return N * M; }
    
    public:
        constexpr flattened_type flatten() const noexcept { return std::bit_cast<flattened_type>(*this); }
        constexpr explicit operator flattened_type() const noexcept { return flatten(); }

    public:
        template<std::size_t A, std::size_t B, typename U>
        constexpr explicit operator matrix<A, B, U>() const noexcept requires (A != M || B != N || !std::is_same_v<T, U>);

    public:
        //TODO use SIMD (i.e. target_clones)
        consteval static matrix<M, N, T> identity() noexcept requires (M == N);
        constexpr static matrix<M, N, T> transposed(matrix<N, M, T> original) noexcept;


        constexpr static matrix<M, N, T> scaling(std::array<T, N-1> scale_vec) noexcept requires (M == N);
        template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<std::remove_cvref_t<A>>
        constexpr static matrix<M, N, T> rotating(A&& angle, FS&& sin_fn, FC&& cos_fn) noexcept requires (M == N && N == 2);
        template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<std::remove_cvref_t<A>>
        constexpr static matrix<M, N, T> rotating(A&& angle, axis rotate_axis, FS&& sin_fn, FC&& cos_fn) noexcept requires (M == N && N == 3);
        template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<std::remove_cvref_t<A>>
        constexpr static matrix<M, N, T> rotating(A&& angle, axis rotate_axis, FS&& sin_fn, FC&& cos_fn) noexcept requires (M == N && N == 4);
        constexpr static matrix<M, N, T> translating(std::array<T, N-1> translate_vec) noexcept requires (M == N);

        template<typename F>
        constexpr static matrix<M, N, T> looking_at(vector<3, T> eye, vector<3, T> target, axis up_axis, axis_direction dir, F&& sqrt_fn) noexcept requires (M == N && N == 4);
        template<typename A, typename F>
        constexpr static matrix<M, N, T> perspective(A fov_angle, T screen_width, T screen_height, F&& tan_fn, T near_z, T far_z) noexcept requires (M == N && N == 4);
        template<typename A, typename F>
        constexpr static matrix<M, N, T> perspective(A fov_angle, T screen_width, T screen_height, F&& tan_fn, T near_z) noexcept requires (M == N && N == 4);
        constexpr static matrix<M, N, T> orthographic(T screen_width, T screen_height, T near_z, T far_z) noexcept requires (M == N && N == 4);
        constexpr static matrix<M, N, T> orthographic(T screen_width, T screen_height) noexcept requires (M == N && N == 4);


    public:
        //TODO use SIMD (i.e. target_clones)
        friend constexpr column_type operator*(matrix<M, N, T> lhs, const column_type& rhs) noexcept {
            return mult(lhs, rhs, std::make_index_sequence<M>{}, sl::size_constant<N>); 
        }
        friend constexpr row_type operator*(row_type lhs, const matrix<M, N, T>& rhs) noexcept {
            return mult(lhs, rhs, sl::size_constant<M>, std::make_index_sequence<N>{});
        }
        template<std::size_t P>
        friend constexpr matrix<M, P, T> operator*(matrix<M, N, T> lhs, const matrix<N, P, T>& rhs) noexcept {
            return mult(lhs, rhs, std::make_index_sequence<M>{}, sl::size_constant<N>, std::make_index_sequence<P>{});
        }

    private:
        template<typename Mat, typename Vec, std::size_t I, std::size_t... Js>
        constexpr static T mult_row(Mat&& m, Vec&& v, sl::size_constant_type<I>, std::index_sequence<Js...>) noexcept;
        template<typename Mat, typename Vec, std::size_t... Is, std::size_t J>
        constexpr static column_type mult(Mat&& m, Vec&& v, std::index_sequence<Is...>, sl::size_constant_type<J>) noexcept;

        template<typename Vec, typename Mat, std::size_t... Is, std::size_t J>
        constexpr static T mult_col(Vec&& v, Mat&& m, std::index_sequence<Is...>, sl::size_constant_type<J>) noexcept;
        template<typename Vec, typename Mat, std::size_t I, std::size_t... Js>
        constexpr static row_type mult(Vec&& v, Mat&& m, sl::size_constant_type<I>, std::index_sequence<Js...>) noexcept;

        template<typename MatA, typename MatB,  std::size_t I, std::size_t... Ks, std::size_t J>
        constexpr static T mult_mat(MatA&& m_a, MatB&& m_b, sl::size_constant_type<I>, std::index_sequence<Ks...>, sl::size_constant_type<J>) noexcept;
        template<typename MatA, typename MatB, std::size_t I, std::size_t K, std::size_t... Js>
        constexpr static row_type mult_mat_cols(MatA&& m_a, MatB&& m_b, sl::size_constant_type<I> i, sl::size_constant_type<K>, std::index_sequence<Js...>) noexcept;
        template<typename MatA, typename MatB, std::size_t... Is, std::size_t K, std::size_t... Js>
        constexpr static matrix<sizeof...(Is), sizeof...(Js), T> mult(MatA&& m_a, MatB&& m_b, std::index_sequence<Is...>, sl::size_constant_type<K>, std::index_sequence<Js...>) noexcept;
    };
}


namespace d2d {
    template<std::size_t N, typename T> class scale;
    template<typename A, typename FS, typename FC> requires std::is_arithmetic_v<A> class rotate;
    template<std::size_t N, typename T> class translate;

    //TODO flatten/target_clones
    template<std::size_t Dims, typename UnitTy, typename... Args>
    constexpr point<Dims, UnitTy> transform(point<Dims, UnitTy> src_vec, Args&&... args) noexcept;
}



namespace d2d {
    template<std::size_t M, std::size_t N, typename T>
    using mat = matrix<M, N, T>;

    template<typename T> using mat2 = matrix<2, 2, T>;
    template<typename T> using mat3 = matrix<3, 3, T>;
    template<typename T> using mat4 = matrix<4, 4, T>;


    template<std::size_t N> 
    using vk_mat = matrix<N, N, float>;

    using vk_mat2 = vk_mat<2>;
    using vk_mat3 = vk_mat<3>;
    using vk_mat4 = vk_mat<4>;
}

#include "Duo2D/arith/matrix.inl"