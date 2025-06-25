#pragma once
#include <filesystem>
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

        constexpr bool empty() const noexcept { return first.empty() || second.empty(); }
    };
}