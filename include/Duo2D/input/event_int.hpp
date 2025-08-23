#pragma once
#include <bit>
#include <compare>
#include <cstddef>
#include <cstdint>


namespace d2d::input{
    //TODO: assumes size_t is 64 bits wide
    using event_id_t = std::int32_t;
    using category_id_t = std::uint32_t;
}


namespace d2d::input {
    //Any padding bits causes the equality and hash to be incorrect due to the indeterminate bits
    struct /*alignas(std::size_t)*/ event_t {
        event_id_t event_id;
        category_id_t category_id;
    };
}



namespace std {
    template<class Key>
    struct hash;
}

namespace std {
    template<>
    struct hash<d2d::input::event_t> {
        constexpr std::size_t operator()(d2d::input::event_t event) const noexcept {
            return std::bit_cast<std::size_t>(event);
        }
    };
}

namespace d2d::input {
    constexpr bool operator==(const event_t& lhs, const event_t& rhs) noexcept {
        return std::bit_cast<std::size_t>(lhs) == std::bit_cast<std::size_t>(rhs);
    }
    constexpr std::strong_ordering operator<=>(const event_t& lhs, const event_t& rhs) noexcept {
        return std::bit_cast<std::size_t>(lhs) <=> std::bit_cast<std::size_t>(rhs);
    }
}