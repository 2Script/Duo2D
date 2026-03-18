#pragma once
#include <bit>
#include <limits.h>
#include <streamline/numeric/int.hpp>

#include "sirius/core/buffer_key_t.hpp"


namespace acma {
    using buffer_category_t = sl::uint16_t;
    using buffer_local_id_t = buffer_key_t;

	constexpr sl::size_t buffer_local_id_size_bits = (sizeof(buffer_key_t) - sizeof(buffer_category_t)) * CHAR_BIT;
}

namespace acma {
    struct alignas(buffer_key_t) buffer_id {
        buffer_category_t category;
        buffer_local_id_t local_id : buffer_local_id_size_bits;

        constexpr operator buffer_key_t() const noexcept {
            return static_cast<buffer_key_t>(static_cast<buffer_key_t>(category) << buffer_local_id_size_bits | local_id);
            //return std::bit_cast<std::uint64_t>(*this);
        }
    };
}