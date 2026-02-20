#pragma once
#include <bit>
#include <cstdint>


namespace d2d {
    using resource_type_id = std::uint16_t;

    namespace resource_type_ids {
        constexpr resource_type_id global = 0;
    }
}

namespace d2d {
    struct alignas(std::uint64_t) resource_id {
        resource_type_id type_id;
        std::uint64_t buffer_id : 48;

        constexpr operator std::uint64_t() const noexcept {
            return static_cast<std::uint64_t>(type_id) << 48 | buffer_id;
            //return std::bit_cast<std::uint64_t>(*this);
        }
    };
}

namespace d2d {
    namespace resource_ids {
        constexpr resource_id textures         = {resource_type_ids::global, (static_cast<std::uint64_t>(-1) >> 16) - 0};
        constexpr resource_id texture_indicies = {resource_type_ids::global, (static_cast<std::uint64_t>(-1) >> 16) - 1};
        constexpr resource_id buffer_addresses = {resource_type_ids::global, (static_cast<std::uint64_t>(-1) >> 16) - 2};

        constexpr std::size_t count = 3;
    }
}