#pragma once
#include "Duo2D/arith/vector.hpp"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <type_traits>

namespace d2d {
    template<std::size_t Dims, typename UnitTy> using size = vector<Dims, UnitTy, impl::vec_data_type::size>;
}

namespace d2d {
    template<typename UnitTy> using size2 = size<2, UnitTy>;
    template<typename UnitTy> using size3 = size<3, UnitTy>;
    template<typename UnitTy> using sz2 = size<2, UnitTy>;
    template<typename UnitTy> using sz3 = size<3, UnitTy>;
}

namespace d2d {
    using size2f  = size2<float>;
    using size3f  = size3<float>;
    using sz2f    = size2<float>;
    using sz3f    = size3<float>;

    using size2d  = size2<double>;
    using size3d  = size3<double>;
    using sz2d    = size2<double>;
    using sz3d    = size3<double>;


    using size2u8 = size2<std::uint8_t>;
    using size3u8 = size3<std::uint8_t>;
    using sz2u8   = size2<std::uint8_t>;
    using sz3u8   = size3<std::uint8_t>;

    using size2u16 = size2<std::uint16_t>;
    using size3u16 = size3<std::uint16_t>;
    using sz2u16   = size2<std::uint16_t>;
    using sz3u16   = size3<std::uint16_t>;

    using size2u32 = size2<std::uint32_t>;
    using size3u32 = size3<std::uint32_t>;
    using sz2u32   = size2<std::uint32_t>;
    using sz3u32   = size3<std::uint32_t>;

    using size2u64 = size2<std::uint64_t>;
    using size3u64 = size3<std::uint64_t>;
    using sz2u64   = size2<std::uint64_t>;
    using sz3u64   = size3<std::uint64_t>;

    using size2s8 = size2<std::int8_t>;
    using size3s8 = size3<std::int8_t>;
    using sz2s8   = size2<std::int8_t>;
    using sz3s8   = size3<std::int8_t>;

    using size2s16 = size2<std::int16_t>;
    using size3s16 = size3<std::int16_t>;
    using sz2s16   = size2<std::int16_t>;
    using sz3s16   = size3<std::int16_t>;

    using size2s32 = size2<std::int32_t>;
    using size3s32 = size3<std::int32_t>;
    using sz2s32   = size2<std::int32_t>;
    using sz3s32   = size3<std::int32_t>;

    using size2s64 = size2<std::int64_t>;
    using size3s64 = size3<std::int64_t>;
    using sz2s64   = size2<std::int64_t>;
    using sz3s64   = size3<std::int64_t>;
}

namespace d2d {
    using extent2 = size2<std::uint32_t>;
    using extent3 = size3<std::uint32_t>;
}