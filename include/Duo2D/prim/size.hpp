#pragma once
#include "Duo2D/prim/vector.hpp"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <type_traits>

namespace d2d {
    template<typename UnitTy>
    struct size2 : vector_base<vector, 2, UnitTy> {
        constexpr explicit operator std::pair<UnitTy, UnitTy>() const noexcept { return {width(), height()}; }

        constexpr explicit operator std::enable_if_t<std::is_convertible_v<UnitTy, std::uint32_t>, VkExtent2D>() const noexcept { 
            return {static_cast<std::uint32_t>(width()), static_cast<std::uint32_t>(height())}; 
        }

        constexpr       UnitTy& width()        noexcept { return this->_elems[0]; }
        constexpr       UnitTy& height()       noexcept { return this->_elems[1]; }
        constexpr const UnitTy& width()  const noexcept { return this->_elems[0]; }
        constexpr const UnitTy& height() const noexcept { return this->_elems[1]; }
    };

    template<typename UnitTy>
    struct size3 : vector_base<vector, 3, UnitTy> {
        constexpr explicit operator std::enable_if_t<std::is_convertible_v<UnitTy, std::uint32_t>, VkExtent3D>() const noexcept { 
            return {static_cast<std::uint32_t>(width()), static_cast<std::uint32_t>(height()), static_cast<std::uint32_t>(depth())}; 
        }

        constexpr       UnitTy& width()        noexcept { return this->_elems[0]; }
        constexpr       UnitTy& height()       noexcept { return this->_elems[1]; }
        constexpr       UnitTy& depth()        noexcept { return this->_elems[2]; }
        constexpr const UnitTy& width()  const noexcept { return this->_elems[0]; }
        constexpr const UnitTy& height() const noexcept { return this->_elems[1]; }
        constexpr const UnitTy& depth()  const noexcept { return this->_elems[2]; }
    };
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