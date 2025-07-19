#pragma once
#include <limits>
#include <string_view>
#include <llfio.hpp>
#include <msdfgen.h>
#include <map>
#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/graphics/core/font_data.hpp"
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/vulkan/memory/texture_map_base.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/traits/instance_tracker.hpp"
#include <harfbuzz/hb.h>

namespace d2d::vk {
    //using path_view = LLFIO_V2_NAMESPACE::path_view;

    class texture_map : public texture_map_base {
    public:
        using font_bytes_map_type = std::map<std::string_view, std::vector<std::byte>>;
    public:
        //TODO (HIGH PRIO): Split this into (multithreaded) loading/decoding the file | allocating multiple files into image buffers
        result<texture_idx_t> load(std::string_view path, std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_map, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        result<texture_idx_t> load(const font& f,         std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<::d2d::impl::font_data_map> font_map, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        //void unload(std::string_view key) noexcept;
    private:
        result<texture_idx_t> create_texture(
            iterator& tex_iter, std::span<std::span<const std::byte>> bytes, extent2 texture_size, VkFormat format,
            std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem
        ) noexcept;


    private:
        using texture_map_base::insert;
        using texture_map_base::insert_or_assign;
        using texture_map_base::emplace;
        using texture_map_base::emplace_hint;
        using texture_map_base::try_emplace;
        using texture_map_base::swap;
        using texture_map_base::extract;
        using texture_map_base::merge;
        using texture_map_base::at;
        using texture_map_base::operator[];

    public:
        constexpr static std::size_t font_texture_length_pixels = 32;
        constexpr static std::size_t font_texture_channels = 4;
        constexpr static std::size_t font_texture_size_bytes = font_texture_length_pixels * font_texture_length_pixels * font_texture_channels;
        constexpr static double font_texture_padding_em = 0.0625;
        constexpr static double font_texture_distance_range = 0.125;
        constexpr static std::size_t font_texture_glyph_scale = font_texture_length_pixels;

    private:
        struct unicode_variation_pair {
            hb_codepoint_t codepoint;
            hb_codepoint_t variation;
        };
    };
}

namespace d2d::vk::impl {
    struct glyph_context {
        pt2d pos;
        double scale;
    };
}

namespace d2d::vk::impl::draw_op {
    constexpr void move_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    constexpr void line_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept;
    constexpr void quad_to (hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float control_x, float control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
    constexpr void cubic_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float first_control_x, float first_control_y, float second_control_x, float second_control_y, float target_x, float target_y, void* glyph_ctx) noexcept;
}

#include "Duo2D/vulkan/memory/texture_map.inl"