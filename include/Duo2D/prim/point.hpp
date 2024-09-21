#pragma once
#include "Duo2D/prim/vector.hpp"
#include <cstdint>

namespace d2d {
    template<std::size_t Dims, typename UnitTy>
    using point = vector<Dims, UnitTy>;
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
    using point2f = point<2, float>;
    using point3f = point<3, float>;
    using pt2f    = point<2, float>;
    using pt3f    = point<3, float>;

    using point2d = point<2, double>;
    using point3d = point<3, double>;
    using pt2d    = point<2, double>;
    using pt3d    = point<3, double>;

    using point2i = point<2, std::int64_t>;
    using point3i = point<3, std::int64_t>;
    using pt2i    = point<2, std::int64_t>;
    using pt3i    = point<3, std::int64_t>;

    using point2u = point<2, std::uint64_t>;
    using point3u = point<3, std::uint64_t>;
    using pt2u    = point<2, std::uint64_t>;
    using pt3u    = point<3, std::uint64_t>;
}