#pragma once
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <string>
#include <utility>

namespace d2d {
    struct font : public std::pair<std::string, std::filesystem::path> {
        constexpr std::string const&  name() const&  noexcept { return first; }
        constexpr std::string      &  name()      &  noexcept { return first; }
        constexpr std::string const&& name() const&& noexcept { return std::move(first); }
        constexpr std::string      && name()      && noexcept { return std::move(first); }

        constexpr std::filesystem::path const&  path() const&  noexcept { return second; }
        constexpr std::filesystem::path      &  path()      &  noexcept { return second; }
        constexpr std::filesystem::path const&& path() const&& noexcept { return std::move(second); }
        constexpr std::filesystem::path      && path()      && noexcept { return std::move(second); }

    public:
        constexpr bool empty() const noexcept { return first.empty() || second.empty(); }

    public:
        constexpr std::string key() const noexcept { return std::format("font::{}", first); }
    
    public:
        constexpr static unsigned char nonprintable_ascii_count = 0x1F + 1;
        constexpr static unsigned char total_ascii_count = std::numeric_limits<char>::max() + 1;
        constexpr static unsigned char printable_ascii_count = total_ascii_count - nonprintable_ascii_count;
        constexpr static std::uint_fast32_t unicode_count = 0x1FFFFF + 1;
    };
}
