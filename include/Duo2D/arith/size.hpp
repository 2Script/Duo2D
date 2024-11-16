#pragma once
#include "Duo2D/arith/vector.hpp"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <type_traits>

namespace d2d {
    template<std::size_t Dims, typename UnitTy> using size = vector<Dims, UnitTy, true>;
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

    using size2i  = size2<std::int64_t>;
    using size3i  = size3<std::int64_t>;
    using sz2i    = size2<std::int64_t>;
    using sz3i    = size3<std::int64_t>;

    using size2u  = size2<std::uint64_t>;
    using size3u  = size3<std::uint64_t>;
    using sz2u    = size2<std::uint64_t>;
    using sz3u    = size3<std::uint64_t>;
}

namespace d2d {
    using extent2 = size2<std::uint32_t>;
    using extent3 = size3<std::uint32_t>;
}