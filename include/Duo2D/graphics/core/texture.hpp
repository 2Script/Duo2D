#pragma once
#include <string>
#include <string_view>

namespace d2d {
    class texture_view : public std::string_view {
    public:
        using std::string_view::basic_string_view;
    };
}

namespace d2d {
    class texture : public std::string {
    public:
        using std::string::basic_string;
    public:
        constexpr operator texture_view() const noexcept { return texture_view{data(), size()}; }
    };
}
