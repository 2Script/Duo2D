#pragma once
#include <cstddef>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/prim/color.hpp"
#include "Duo2D/traits/vector_traits.hpp"


namespace d2d::impl {
    template<VkFormat V> using format_constant = std::integral_constant<VkFormat, V>;
    template<std::size_t V> using size_constant = std::integral_constant<std::size_t, V>;
    template<typename T> struct format_of : format_constant<VK_FORMAT_UNDEFINED> {};
    template<typename T> struct location_size_of : size_constant<1> {};

    template<typename T, std::size_t LocationSize = 1>
    struct shader_input_traits {
        constexpr static VkFormat format = format_of<std::remove_cvref_t<T>>::value;
        constexpr static std::size_t location_size = location_size_of<std::remove_cvref_t<T>>::value;
    };
}

/*
namespace d2d::impl {
    template<typename T>
    concept shader_input_like = requires {
        shader_input_traits<T>::format;
        shader_input_traits<T>::location_size;
    };
}
*/


//Fundamental Types
namespace d2d::impl {
    //TODO: rename to fixed_integral and fixed_floating, respectively
    template<typename T, std::size_t Size, bool Signed>
    concept Integral = std::is_integral_v<T> && sizeof(T) == Size && std::is_signed_v<T> == Signed;
    template<typename T, std::size_t Size>
    concept Floating = std::is_floating_point_v<T> && sizeof(T) == Size;

    template<Integral<1, true> T> struct format_of<T> : format_constant<VK_FORMAT_R8_SINT> {};
    template<Integral<2, true> T> struct format_of<T> : format_constant<VK_FORMAT_R16_SINT> {};
    template<Integral<4, true> T> struct format_of<T> : format_constant<VK_FORMAT_R32_SINT> {};
    template<Integral<8, true> T> struct format_of<T> : format_constant<VK_FORMAT_R64_SINT> {};
    template<Integral<1, false> T> struct format_of<T> : format_constant<VK_FORMAT_R8_UINT> {};
    template<Integral<2, false> T> struct format_of<T> : format_constant<VK_FORMAT_R16_UINT> {};
    template<Integral<4, false> T> struct format_of<T> : format_constant<VK_FORMAT_R32_UINT> {};
    template<Integral<8, false> T> struct format_of<T> : format_constant<VK_FORMAT_R64_UINT> {};

    template<Floating<2> T> struct format_of<T> : format_constant<VK_FORMAT_R16_SFLOAT> {};
    template<Floating<4> T> struct format_of<T> : format_constant<VK_FORMAT_R32_SFLOAT> {};
    template<Floating<8> T> struct format_of<T> : format_constant<VK_FORMAT_R64_SFLOAT> {};
}


//Vector types
namespace d2d::impl {
    template<Floating<2> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R16_SFLOAT> {};
    template<Floating<2> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R16G16_SFLOAT> {};
    template<Floating<2> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R16G16B16_SFLOAT> {};
    template<Floating<2> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R16G16B16A16_SFLOAT> {};

    template<Floating<4> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R32_SFLOAT> {};
    template<Floating<4> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R32G32_SFLOAT> {};
    template<Floating<4> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R32G32B32_SFLOAT> {};
    template<Floating<4> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R32G32B32A32_SFLOAT> {};

    template<Floating<8> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R64_SFLOAT> {};
    template<Floating<8> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R64G64_SFLOAT> {};
    template<Floating<8> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R64G64B64_SFLOAT> {};
    template<Floating<8> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R64G64B64A64_SFLOAT> {};


    template<Integral<1, true> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R8_SINT> {};
    template<Integral<1, true> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R8G8_SINT> {};
    template<Integral<1, true> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R8G8B8_SINT> {};
    template<Integral<1, true> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R8G8B8A8_SINT> {};

    template<Integral<2, true> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R16_SINT> {};
    template<Integral<2, true> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R16G16_SINT> {};
    template<Integral<2, true> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R16G16B16_SINT> {};
    template<Integral<2, true> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R16G16B16A16_SINT> {};
    
    template<Integral<4, true> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R32_SINT> {};
    template<Integral<4, true> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R32G32_SINT> {};
    template<Integral<4, true> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R32G32B32_SINT> {};
    template<Integral<4, true> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R32G32B32A32_SINT> {};
    
    template<Integral<8, true> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R64_SINT> {};
    template<Integral<8, true> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R64G64_SINT> {};
    template<Integral<8, true> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R64G64B64_SINT> {};
    template<Integral<8, true> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R64G64B64A64_SINT> {};


    template<Integral<1, false> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R8_UINT> {};
    template<Integral<1, false> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R8G8_UINT> {};
    template<Integral<1, false> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R8G8B8_UINT> {};
    template<Integral<1, false> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R8G8B8A8_UINT> {};

    template<Integral<2, false> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R16_UINT> {};
    template<Integral<2, false> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R16G16_UINT> {};
    template<Integral<2, false> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R16G16B16_UINT> {};
    template<Integral<2, false> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R16G16B16A16_UINT> {};
    
    template<Integral<4, false> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R32_UINT> {};
    template<Integral<4, false> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R32G32_UINT> {};
    template<Integral<4, false> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R32G32B32_UINT> {};
    template<Integral<4, false> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R32G32B32A32_UINT> {};
    
    template<Integral<8, false> T, impl::vec_data_type DT> struct format_of<vector<1, T, DT>> : format_constant<VK_FORMAT_R64_UINT> {};
    template<Integral<8, false> T, impl::vec_data_type DT> struct format_of<vector<2, T, DT>> : format_constant<VK_FORMAT_R64G64_UINT> {};
    template<Integral<8, false> T, impl::vec_data_type DT> struct format_of<vector<3, T, DT>> : format_constant<VK_FORMAT_R64G64B64_UINT> {};
    template<Integral<8, false> T, impl::vec_data_type DT> struct format_of<vector<4, T, DT>> : format_constant<VK_FORMAT_R64G64B64A64_UINT> {};


    template<std::size_t C, std::size_t B> struct format_of<basic_color<C, B>> : format_of<vector<C, typename basic_color<C, B>::component_type>> {};
}


//Matrix types
namespace d2d::impl {
    template<Floating<2> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R16G16B16A16_SFLOAT> {};
    template<Floating<4> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R32G32B32A32_SFLOAT> {};
    template<Floating<8> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R64G64B64A64_SFLOAT> {};
    template<Floating<2> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Floating<4> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Floating<8> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};

    template<Integral<1, true> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R8G8B8A8_SINT    > {};
    template<Integral<2, true> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R16G16B16A16_SINT> {};
    template<Integral<4, true> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R32G32B32A32_SINT> {};
    template<Integral<8, true> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R64G64B64A64_SINT> {};
    template<Integral<1, true> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<2, true> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<4, true> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<8, true> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};

    template<Integral<1, false> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R8G8B8A8_UINT    > {};
    template<Integral<2, false> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R16G16B16A16_UINT> {};
    template<Integral<4, false> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R32G32B32A32_UINT> {};
    template<Integral<8, false> T> struct format_of<matrix<2, 2, T>> : format_constant<VK_FORMAT_R64G64B64A64_UINT> {};
    template<Integral<1, false> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<2, false> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<4, false> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
    template<Integral<8, false> T> struct location_size_of<matrix<2, 2, T>> : size_constant<2> {};
}