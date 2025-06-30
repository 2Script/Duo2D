#pragma once
#include <string_view>
#include <llfio.hpp>
#include <msdfgen.h>
#include <map>
#include "Duo2D/arith/point.hpp"
#include "Duo2D/graphics/core/font.hpp"
#include "Duo2D/traits/generic_functor.hpp"
#include "Duo2D/vulkan/memory/texture_map_base.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/core/error.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/traits/instance_tracker.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace d2d::vk {
    //using path_view = LLFIO_V2_NAMESPACE::path_view;

    class texture_map : public texture_map_base {
    public:
        //TODO (HIGH PRIO): Split this into (multithreaded) loading/decoding the file | allocating multiple files into image buffers
        result<texture_idx_t> load(std::string_view path, std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        result<texture_idx_t> load(const font& f,         std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem) noexcept;
        //void unload(std::string_view key) noexcept;
    private:
        result<texture_idx_t> create_texture(
            iterator& tex_iter, std::span<std::span<const std::byte>> bytes, extent2 texture_size, VkFormat format,
            std::shared_ptr<logical_device> logi_device, std::shared_ptr<physical_device> phys_device, std::shared_ptr<command_pool> copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem
        ) noexcept;
    
    private:
        using face_ptr = std::unique_ptr<std::remove_pointer_t<FT_Face>, generic_functor<FT_Done_Face>>;
        struct freetype_context {
            pt2d pos;
            double scale;
            msdfgen::Shape& shape;
        };

    private:
        ::d2d::impl::instance_tracker<FT_Library, FT_Done_FreeType> freetype_init{};
        std::map<std::string_view, face_ptr> faces;

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

namespace d2d::vk::impl {
    inline std::atomic_int64_t& texture_map_count() {
        static std::atomic_int64_t count{};
        return count;
    }
}