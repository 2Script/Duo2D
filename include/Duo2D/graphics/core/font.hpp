#pragma once
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <span>
#include <string_view>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <vector>
#include <cstddef>
#include <memory>

#include <harfbuzz/hb.h>

#include "Duo2D/arith/rect.hpp"
#include "Duo2D/traits/generic_functor.hpp"


namespace d2d {
    class font_view : public std::string_view {
    public:
        using std::string_view::basic_string_view;
    };

    class font : public std::string {
    public:
        constexpr font() noexcept = default;
        
    public:
        [[gnu::nonnull]] constexpr font(char const* s) noexcept : std::string(std::format(font_key_format_string, s)) {}
        [[gnu::nonnull]] constexpr font(char const* s, std::size_t count) noexcept : std::string(std::format(font_key_format_string, std::string_view(s, count))) {}
        
        template<typename StringViewLike> requires (std::is_convertible_v<StringViewLike const&, std::string_view> && !std::is_convertible_v<StringViewLike const&, char const*>)
        constexpr explicit font(StringViewLike const& s) noexcept : std::string(std::format(font_key_format_string, std::string_view(s))) {}
        template<typename StringViewLike> requires std::is_convertible_v<StringViewLike const&, std::string_view>
        constexpr font(StringViewLike const& s, std::size_t pos, std::size_t count) noexcept : std::string(std::format(font_key_format_string, std::string_view(s).substr(pos, count))) {}
    public:
        constexpr operator font_view() const noexcept { return font_view{data(), size()}; }


    public:
        constexpr static unsigned char nonprintable_ascii_count = 0x1F + 1;
        constexpr static unsigned char total_ascii_count = std::numeric_limits<char>::max() + 1;
        constexpr static unsigned char printable_ascii_count = total_ascii_count - nonprintable_ascii_count;
        constexpr static std::uint_fast32_t unicode_count = 0x1FFFFF + 1;
    private:
        constexpr static std::string_view font_key_format_string = "font::{}";
    };
}

namespace d2d {
    using font_size_t = std::uint_fast32_t;
}



namespace d2d::impl {
    struct font_data {
        font_data() noexcept = default;
        
        inline font_data(std::span<const std::byte> bytes_span) :
            font_file_bytes(bytes_span.begin(), bytes_span.end()), glyph_bounds(), glyph_padding(),
            blob_ptr(hb_blob_create(reinterpret_cast<char*>(font_file_bytes.data()), font_file_bytes.size(), HB_MEMORY_MODE_WRITABLE, nullptr, nullptr)),
            face_ptr(hb_face_create(blob_ptr.get(), 0)), 
            font_ptr(hb_font_create(face_ptr.get())) {}

    public:
        std::vector<std::byte> font_file_bytes;
        std::vector<rect<float>> glyph_bounds;
        std::vector<pt2f> glyph_padding;

        std::unique_ptr<hb_blob_t, generic_functor<hb_blob_destroy>> blob_ptr;
        std::unique_ptr<hb_face_t, generic_functor<hb_face_destroy>> face_ptr;
        std::unique_ptr<hb_font_t, generic_functor<hb_font_destroy>> font_ptr;
    };

    using font_data_map = std::unordered_map<std::string, font_data>;
}


namespace d2d::impl {
    using font_path_map = std::map<font, std::filesystem::path>;
}
