#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <memory>
#include <harfbuzz/hb.h>
#include "Duo2D/arith/rect.hpp"
#include "Duo2D/traits/generic_functor.hpp"

namespace d2d {
    using glyph_idx_t = std::uint_least16_t;
    using unicode_cp_t = hb_codepoint_t;
}

namespace d2d::impl {
    using glyph_bounds_vector = std::vector<rect<float>>;
    using glyph_index_map = std::unordered_map<unicode_cp_t, glyph_idx_t>;
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
        glyph_bounds_vector glyph_bounds;
        std::vector<pt2f> glyph_padding;

        std::unique_ptr<hb_blob_t, generic_functor<hb_blob_destroy>> blob_ptr;
        std::unique_ptr<hb_face_t, generic_functor<hb_face_destroy>> face_ptr;
        std::unique_ptr<hb_font_t, generic_functor<hb_font_destroy>> font_ptr;
    };

    using font_data_map = std::unordered_map<std::string, font_data>;
}