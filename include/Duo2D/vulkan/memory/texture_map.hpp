#pragma once
#include <string_view>
#include <llfio.hpp>
#include <msdfgen.h>
#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/vulkan/memory/texture_map_base.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/graphics/core/texture.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/core/instance_tracker.hpp"

namespace d2d {
    //using path_view = LLFIO_V2_NAMESPACE::path_view;

    class texture_map : public texture_map_base {
    public:
        //TODO (HIGH PRIO): Split this into (multithreaded) loading/decoding the file | allocating multiple files into image buffers
        result<texture_idx_t> load(std::string_view path, logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        result<texture_idx_t> load(const font& f,         logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        //void unload(std::string_view key) noexcept;
    private:
        result<texture_idx_t> create_texture(
            iterator& tex_iter, std::span<std::span<const std::byte>> bytes, extent2 texture_size, VkFormat format,
            logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem
        ) noexcept;

    private:
        impl::instance_tracker<FT_Library, FT_Done_FreeType> freetype_init{};
    
    private:
        struct freetype_context {
            pt2d pos;
            double scale;
            msdfgen::Shape& shape;
        };

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
    };
}

namespace d2d::impl {
    inline std::atomic_int64_t& texture_map_count() {
        static std::atomic_int64_t count{};
        return count;
    }
}