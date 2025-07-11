#pragma once
#include <climits>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/matrix.hpp"
#include "Duo2D/arith/vector.hpp"
#include "Duo2D/graphics/core/color.hpp"
#include "Duo2D/traits/size_constant.hpp"
#include "Duo2D/traits/vector_traits.hpp"


namespace d2d::vk::impl {
    template<VkFormat V> using format_constant = std::integral_constant<VkFormat, V>;
    template<typename T> struct location_size_of : size_constant<1> {};
    template<typename T> struct format_of : format_constant<VK_FORMAT_UNDEFINED> {
        static_assert(!std::is_same_v<T, T>, "Missing format specialization for T"); 
    };

    template<typename T, std::size_t LocationSize = 1>
    struct shader_input_traits {
        constexpr static VkFormat format = format_of<std::remove_cvref_t<T>>::value;
        constexpr static std::size_t location_size = location_size_of<std::remove_cvref_t<T>>::value;
    };
}

/*
namespace d2d::vk::impl {
    template<typename T>
    concept shader_input_like = requires {
        shader_input_traits<T>::format;
        shader_input_traits<T>::location_size;
    };
}
*/

namespace d2d::vk::impl {
    template<typename T, std::size_t Size> concept fixed_signed_integral   = std::signed_integral<T>   && sizeof(T) * CHAR_BIT == Size;
    template<typename T, std::size_t Size> concept fixed_unsigned_integral = std::unsigned_integral<T> && sizeof(T) * CHAR_BIT == Size;
    template<typename T, std::size_t Size> concept fixed_floating_point    = std::floating_point<T>    && sizeof(T) * CHAR_BIT == Size;
}


//Fundamental Types
namespace d2d::vk::impl {
    template<fixed_floating_point<16>    T> struct format_of<T> : format_constant<VK_FORMAT_R16_SFLOAT> {};
    template<fixed_floating_point<32>    T> struct format_of<T> : format_constant<VK_FORMAT_R32_SFLOAT> {};
    template<fixed_floating_point<64>    T> struct format_of<T> : format_constant<VK_FORMAT_R64_SFLOAT> {};

    template<fixed_signed_integral< 8>   T> struct format_of<T> : format_constant<VK_FORMAT_R8_SINT   > {};
    template<fixed_signed_integral<16>   T> struct format_of<T> : format_constant<VK_FORMAT_R16_SINT  > {};
    template<fixed_signed_integral<32>   T> struct format_of<T> : format_constant<VK_FORMAT_R32_SINT  > {};
    template<fixed_signed_integral<64>   T> struct format_of<T> : format_constant<VK_FORMAT_R64_SINT  > {};

    template<fixed_unsigned_integral< 8> T> struct format_of<T> : format_constant<VK_FORMAT_R8_UINT   > {};
    template<fixed_unsigned_integral<16> T> struct format_of<T> : format_constant<VK_FORMAT_R16_UINT  > {};
    template<fixed_unsigned_integral<32> T> struct format_of<T> : format_constant<VK_FORMAT_R32_UINT  > {};
    template<fixed_unsigned_integral<64> T> struct format_of<T> : format_constant<VK_FORMAT_R64_UINT  > {};
}


//Vector types
namespace d2d::vk::impl {
    template<fixed_floating_point<16>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R16_SFLOAT> {};
    template<fixed_floating_point<16>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16_SFLOAT> {};
    template<fixed_floating_point<16>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16_SFLOAT> {};
    template<fixed_floating_point<16>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16A16_SFLOAT> {};

    template<fixed_floating_point<32>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R32_SFLOAT> {};
    template<fixed_floating_point<32>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32_SFLOAT> {};
    template<fixed_floating_point<32>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32_SFLOAT> {};
    template<fixed_floating_point<32>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32A32_SFLOAT> {};

    template<fixed_floating_point<64>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R64_SFLOAT> {};
    template<fixed_floating_point<64>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64_SFLOAT> {};
    template<fixed_floating_point<64>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64_SFLOAT> {};
    template<fixed_floating_point<64>    UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64A64_SFLOAT> {};


    template<fixed_signed_integral< 8>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R8_SINT> {};
    template<fixed_signed_integral< 8>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8_SINT> {};
    template<fixed_signed_integral< 8>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8B8_SINT> {};
    template<fixed_signed_integral< 8>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8B8A8_SINT> {};

    template<fixed_signed_integral<16>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R16_SINT> {};
    template<fixed_signed_integral<16>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16_SINT> {};
    template<fixed_signed_integral<16>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16_SINT> {};
    template<fixed_signed_integral<16>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16A16_SINT> {};
    
    template<fixed_signed_integral<32>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R32_SINT> {};
    template<fixed_signed_integral<32>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32_SINT> {};
    template<fixed_signed_integral<32>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32_SINT> {};
    template<fixed_signed_integral<32>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32A32_SINT> {};
    
    template<fixed_signed_integral<64>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R64_SINT> {};
    template<fixed_signed_integral<64>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64_SINT> {};
    template<fixed_signed_integral<64>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64_SINT> {};
    template<fixed_signed_integral<64>   UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64A64_SINT> {};


    template<fixed_unsigned_integral< 8> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R8_UINT> {};
    template<fixed_unsigned_integral< 8> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8_UINT> {};
    template<fixed_unsigned_integral< 8> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8B8_UINT> {};
    template<fixed_unsigned_integral< 8> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R8G8B8A8_UINT> {};

    template<fixed_unsigned_integral<16> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R16_UINT> {};
    template<fixed_unsigned_integral<16> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16_UINT> {};
    template<fixed_unsigned_integral<16> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16_UINT> {};
    template<fixed_unsigned_integral<16> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R16G16B16A16_UINT> {};
    
    template<fixed_unsigned_integral<32> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R32_UINT> {};
    template<fixed_unsigned_integral<32> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32_UINT> {};
    template<fixed_unsigned_integral<32> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32_UINT> {};
    template<fixed_unsigned_integral<32> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R32G32B32A32_UINT> {};
    
    template<fixed_unsigned_integral<64> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<1, UnitT, DataT>> : format_constant<VK_FORMAT_R64_UINT> {};
    template<fixed_unsigned_integral<64> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<2, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64_UINT> {};
    template<fixed_unsigned_integral<64> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<3, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64_UINT> {};
    template<fixed_unsigned_integral<64> UnitT, ::d2d::impl::vec_data_type DataT> struct format_of<vector<4, UnitT, DataT>> : format_constant<VK_FORMAT_R64G64B64A64_UINT> {};


    template<std::size_t C, std::size_t B> struct format_of<basic_color<C, B>> : format_of<vector<C, typename basic_color<C, B>::component_type>> {};
}


//Matrix types
namespace d2d::vk::impl {
    template<fixed_floating_point<16>    UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R16G16B16A16_SFLOAT> {};
    template<fixed_floating_point<32>    UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R32G32B32A32_SFLOAT> {};
    template<fixed_floating_point<64>    UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R64G64B64A64_SFLOAT> {};

    template<fixed_signed_integral< 8>   UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R8G8B8A8_SINT    > {};
    template<fixed_signed_integral<16>   UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R16G16B16A16_SINT> {};
    template<fixed_signed_integral<32>   UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R32G32B32A32_SINT> {};
    template<fixed_signed_integral<64>   UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R64G64B64A64_SINT> {};

    template<fixed_unsigned_integral< 8> UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R8G8B8A8_UINT    > {};
    template<fixed_unsigned_integral<16> UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R16G16B16A16_UINT> {};
    template<fixed_unsigned_integral<32> UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R32G32B32A32_UINT> {};
    template<fixed_unsigned_integral<64> UnitT> struct format_of<matrix<2, 2, UnitT>> : format_constant<VK_FORMAT_R64G64B64A64_UINT> {};
    
    template<typename UnitT> struct location_size_of<matrix<2, 2, UnitT>> : size_constant<2> {};
}