#include "Duo2D/vulkan/memory/texture_map.hpp"

#include <result/verify.h>
#include <span>
#include <tuple>
#include <llfio.hpp>
#include <DuoDecode/decoder.hpp>
#include <vulkan/vulkan_core.h>
#include "Duo2D/vulkan/display/pixel_format_mapping.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/make.hpp"

namespace d2d {
    result<texture_idx_t> texture_map::load(std::string_view path, logical_device& logi_device, physical_device& phys_device, command_pool& copy_cmd_pool, device_memory<std::dynamic_extent>& texture_mem, buffer& texture_size_buffer) noexcept {
        auto emplace_result = emplace(std::piecewise_construct, std::forward_as_tuple(path), std::forward_as_tuple());
        texture& tex = emplace_result.first->second;
        if(!emplace_result.second) return tex.index();
        
        namespace llfio = LLFIO_V2_NAMESPACE;
        auto mf = llfio::mapped_file({}, llfio::path_view(path));
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());
        std::size_t length = extent.assume_value();

        dd::decoder image_decoder(mh.address(), length);
        RESULT_TRY_MOVE_UNSCOPED_CAST_ERR(dd::decoded_video decoded_image, image_decoder.decode_video_only(AV_HWDEVICE_TYPE_VULKAN), di, d2d::errc);

        RESULT_TRY_MOVE(tex, make<texture>(
            logi_device, 
            decoded_image.width, decoded_image.height, pixel_format_mapping[decoded_image.format], 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        ));
        if((texture_size_buffer.size() / sizeof(extent2)) < size())
            RESULT_TRY_MOVE(texture_size_buffer, make<buffer>(logi_device, size() * sizeof(extent2), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT));


        renderable_allocator allocator(logi_device, phys_device, copy_cmd_pool);

        std::array<std::span<std::byte>, 1> input = {std::span{decoded_image.bytes}};
        RESULT_VERIFY_UNSCOPED(allocator.stage(decoded_image.bytes.size(), input), s);
        auto [staging_buffer, staging_mem] = *std::move(s);

        std::vector<extent2> texture_size_inputs;
        texture_size_inputs.reserve(size());
        for(auto iter = cbegin(); iter != cend(); ++iter) 
            texture_size_inputs.push_back(iter->second.size());
        RESULT_VERIFY_UNSCOPED(allocator.stage(size() * sizeof(extent2), std::array<std::span<extent2>, 1>{std::span{texture_size_inputs}}), ts);
        auto [texture_size_staging_buffer, texture_size_staging_mem] = *std::move(ts);
        
        //Re-allocate the memory
        device_memory<std::dynamic_extent> old_mem = std::move(texture_mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
        RESULT_TRY_MOVE(texture_mem, make<device_memory<std::dynamic_extent>>(logi_device, phys_device, *this, texture_size_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
         
        //Re-create all other textures
        RESULT_VERIFY(allocator.gpu_alloc_begin());
        std::vector<texture> new_texs;
        new_texs.resize(size());
        RESULT_TRY_COPY_UNSCOPED(std::size_t mem_offset, allocator.realloc(new_texs, *this, texture_mem, std::distance(begin(), emplace_result.first)), realloc);
        RESULT_VERIFY(allocator.gpu_alloc_end());
        RESULT_VERIFY(texture_mem.bind(logi_device, texture_size_buffer, mem_offset));

        //Replace the old textures with the new ones
        allocator.move(*this, new_texs);

        //Move the staging data to the new texture
        RESULT_VERIFY(allocator.staging_to_device_local(tex, staging_buffer));
        RESULT_VERIFY(allocator.staging_to_device_local(texture_size_buffer, texture_size_staging_buffer));

        RESULT_VERIFY(tex.initialize(logi_device, phys_device, pixel_format_mapping[decoded_image.format]));
        return tex.index();
    }
}