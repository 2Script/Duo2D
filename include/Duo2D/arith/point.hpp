#pragma once
#include "Duo2D/arith/vector.hpp"
#include <cstdint>

namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using point = vector<Dims, UnitTy, impl::vec_data_type::point>;
}

namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using pt = point<Dims, UnitTy>;

    template<typename UnitTy> using point2 = point<2, UnitTy>;
    template<typename UnitTy> using point3 = point<3, UnitTy>;
    template<typename UnitTy> using pt2 = point<2, UnitTy>;
    template<typename UnitTy> using pt3 = point<3, UnitTy>;
}

namespace d2d {
    using point2f = point2<float>;
    using point3f = point3<float>;
    using pt2f    = point2<float>;
    using pt3f    = point3<float>;

    using point2d = point2<double>;
    using point3d = point3<double>;
    using pt2d    = point2<double>;
    using pt3d    = point3<double>;


    using point2u8 = point2<std::uint8_t>;
    using point3u8 = point3<std::uint8_t>;
    using pt2u8    = point2<std::uint8_t>;
    using pt3u8    = point3<std::uint8_t>;

    using point2u16 = point2<std::uint16_t>;
    using point3u16 = point3<std::uint16_t>;
    using pt2u16    = point2<std::uint16_t>;
    using pt3u16    = point3<std::uint16_t>;

    using point2u32 = point2<std::uint32_t>;
    using point3u32 = point3<std::uint32_t>;
    using pt2u32    = point2<std::uint32_t>;
    using pt3u32    = point3<std::uint32_t>;

    using point2u64 = point2<std::uint64_t>;
    using point3u64 = point3<std::uint64_t>;
    using pt2u64    = point2<std::uint64_t>;
    using pt3u64    = point3<std::uint64_t>;

    using point2s8 = point2<std::int8_t>;
    using point3s8 = point3<std::int8_t>;
    using pt2s8    = point2<std::int8_t>;
    using pt3s8    = point3<std::int8_t>;

    using point2s16 = point2<std::int16_t>;
    using point3s16 = point3<std::int16_t>;
    using pt2s16    = point2<std::int16_t>;
    using pt3s16    = point3<std::int16_t>;

    using point2s32 = point2<std::int32_t>;
    using point3s32 = point3<std::int32_t>;
    using pt2s32    = point2<std::int32_t>;
    using pt3s32    = point3<std::int32_t>;

    using point2s64 = point2<std::int64_t>;
    using point3s64 = point3<std::int64_t>;
    using pt2s64    = point2<std::int64_t>;
    using pt3s64    = point3<std::int64_t>;
}

namespace d2d {
    using offset2 = point<2, std::int32_t>;
    using offset3 = point<3, std::int32_t>;
}